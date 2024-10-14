#include <re.h>
#include <rem.h>
#include <baresip.h>
#include <sys/time.h>

struct videnc_state {
	struct videnc_param encprm;
	videnc_packet_h *pkth;
	void *arg;
	bool got_key_frame;
	void *key_frame_buf;
	uint32_t key_frame_len;
	bool has_send_frist_frame;

	uint32_t calculate_rate;
	uint32_t calculate_size;
	uint64_t calculate_time;
};

// struct vtx {
// 	struct video *video;               /**< Parent                    */
// 	const struct vidcodec *vc;         /**< Current Video encoder     */
// 	struct videnc_state *enc;          /**< Video encoder state       */
// 	struct vidsrc_prm vsrc_prm;        /**< Video source parameters   */
// 	struct vidsz vsrc_size;            /**< Video source size         */
// 	struct vidsrc_st *vsrc;            /**< Video source              */
// 	struct lock *lock;                 /**< Lock for encoder          */
// 	struct vidframe *frame;            /**< Source frame              */
// 	struct vidframe *mute_frame;       /**< Frame with muted video    */
// 	struct lock *lock_tx;              /**< Protect the sendq         */
// 	struct list sendq;                 /**< Tx-Queue (struct vidqent) */
// 	struct tmr tmr_rtp;                /**< Timer for sending RTP     */
// 	unsigned skipc;                    /**< Number of frames skipped  */
// 	struct list filtl;                 /**< Filters in encoding order */
// 	char device[128];                  /**< Source device name        */
// 	int muted_frames;                  /**< # of muted frames sent    */
// 	uint32_t ts_offset;                /**< Random timestamp offset   */
// 	bool picup;                        /**< Send picture update       */
// 	bool muted;                        /**< Muted flag                */
// 	int frames;                        /**< Number of frames sent     */
// 	int efps;                          /**< Estimated frame-rate      */
// 	uint32_t ts_min;
// 	uint32_t ts_max;
// };

struct vtx {
	struct video *video;               /**< Parent                    */
	const struct vidcodec *vc;         /**< Current Video encoder     */
	struct videnc_state *enc;          /**< Video encoder state       */
	struct vidsrc_prm vsrc_prm;        /**< Video source parameters   */
	struct vidsz vsrc_size;            /**< Video source size         */
	struct vidsrc *vs;                 /**< Video source module       */
	struct vidsrc_st *vsrc;            /**< Video source              */
	mtx_t *lock_enc;                   /**< Lock for encoder          */
	struct vidframe *frame;            /**< Source frame              */
	mtx_t *lock_tx;                    /**< Protect the sendq         */
	struct list sendq;                 /**< Tx-Queue (struct vidqent) */
	struct list sendqnb;               /**< Tx-Queue NACK wait buffer */
	unsigned skipc;                    /**< Number of frames skipped  */
	struct list filtl;                 /**< Filters in encoding order */
	enum vidfmt fmt;                   /**< Outgoing pixel format     */
	char module[128];                  /**< Source module name        */
	char device[128];                  /**< Source device name        */
	uint32_t ts_offset;                /**< Random timestamp offset   */
	bool picup;                        /**< Send picture update       */
	int frames;                        /**< Number of frames sent     */
	double efps;                       /**< Estimated frame-rate      */
	uint64_t ts_base;                  /**< First RTP timestamp sent  */
	uint64_t ts_last;                  /**< Last RTP timestamp sent   */
	thrd_t thrd;                       /**< Tx-Thread                 */
	RE_ATOMIC bool run;                /**< Tx-Thread is active       */
	cnd_t wait;                        /**< Tx-Thread wait            */

	/** Statistics */
	struct {
		uint64_t src_frames;       /**< Total frames from vidsrc  */
	} stats;

	uint32_t ts_min;
	uint32_t ts_max;
};


int32_t g_video_deviation_millisecond = 1000;

int32_t get_video_deviation_millisecond()
{
	return g_video_deviation_millisecond;
}

void set_video_deviation_millisecond(int32_t val)
{
	g_video_deviation_millisecond = val;
}

static void destructor(void *arg) {
	struct videnc_state *vsp = (struct videnc_state *)arg;

	if (vsp->key_frame_buf)
	{
		free(vsp->key_frame_buf);
	}
}

int h264_packet_send(struct vtx *vtx, void *buf, int len)
{
	uint64_t timestamp = 0;
	struct videnc_state *ves = vtx->enc;

	if (ves->has_send_frist_frame) {
		timestamp = 90000ULL * tmr_jiffies() / 1000;
	} else {
		timestamp = 90000ULL * (tmr_jiffies() -  get_video_deviation_millisecond()) / 1000;
		ves->has_send_frist_frame = true;
	}

	if (buf == NULL) return 0;
	int err = h264_packetize(timestamp, buf, len, ves->encprm.pktsize, ves->pkth, ves->arg);
	free(buf);

	return 0;
}

// int h264_encode_update(struct videnc_state **vesp, const struct vidcodec *vc,
// 		struct videnc_param *prm, const char *fmtp,
// 		videnc_packet_h *pkth, void *arg) {
int h264_encode_update(struct videnc_state **vesp,
			  const struct vidcodec *vc, struct videnc_param *prm,
			  const char *fmtp, videnc_packet_h *pkth,
			  const struct video *vid) {
	info("h264_encode_update\n");

	struct vtx *vtx = (struct vtx *)arg;
	struct videnc_state *st;

	if (!vesp || !vc || !prm || !pkth) {
		info("params null");
		return EINVAL;
	}

	st = mem_zalloc(sizeof(*st), destructor);

	st->encprm = *prm;
	st->pkth = pkth;
	st->arg = arg;
	st->has_send_frist_frame = false;

	struct videnc_state *old;
	old = *vesp;
	*vesp = st;

	if (old) {
		mem_deref(old);
	}

	info("h264: video encoder %s: %d fps, %d bit/s, pktsize=%u\n",
		vc->name, prm->fps, prm->bitrate, prm->pktsize);
	return 0;
}

// int h264_encode(struct videnc_state *ves, bool update,
// 		const struct vidframe *frame) {
// 	info("h264_encode\n");
//
// 	uint64_t timestamp = 0;
//
// 	if (ves->has_send_frist_frame) {
// 		timestamp = 90000ULL * tmr_jiffies() / 1000;
// 	} else {
// 		timestamp = 90000ULL * (tmr_jiffies() -  get_video_deviation_millisecond()) / 1000;
// 		ves->has_send_frist_frame = true;
// 	}
//
// 	{
// 		uint8_t *buf = frame->data[0];
// 		int len = frame->linesize[0];
//
// 		if (buf == NULL) return 0;
// 		int err = h264_packetize(timestamp, buf, len, ves->encprm.pktsize, ves->pkth, ves->arg);
// 		free(buf);
// 	}
// 	return 0;
// }
