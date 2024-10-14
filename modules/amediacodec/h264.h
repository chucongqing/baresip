
// struct h264_vidcodec {
// 	struct vidcodec vc;
// };

struct videnc_state;

/* Encode */
// int h264_encode_update(struct videnc_state **vesp, const struct vidcodec *vc,
// 		struct videnc_param *prm, const char *fmtp,
// 		videnc_packet_h *pkth, void *arg);
// int h264_encode(struct videnc_state *ves, bool update,
// 		const struct vidframe *frame);

int h264_encode_update(struct videnc_state **vesp,
			  const struct vidcodec *vc, struct videnc_param *prm,
			  const char *fmtp, videnc_packet_h *pkth,
			  const struct video *vid);
int h264_encode(struct videnc_state *st, bool update,
		   const struct vidframe *frame, uint64_t timestamp);
int h264_packetize(struct videnc_state *st, const struct vidpacket *packet);

/* Decode */
// int h264_decode_update(struct viddec_state **vdsp, const struct vidcodec *vc,
// 		const char *fmtp);
// int h264_decode(struct viddec_state *vds, struct vidframe *frame,
// 		bool *intra, bool marker, uint16_t seq, struct mbuf *mb);

/*
 * Decode
 */

struct viddec_state;

int h264_decode_update(struct viddec_state **vdsp,
			  const struct vidcodec *vc, const char *fmtp,
			  const struct video *vid);
int h264_decode_h264(struct viddec_state *st, struct vidframe *frame,
			struct viddec_packet *pkt);
// int h264_decode_h265(struct viddec_state *st, struct vidframe *frame,
// 			struct viddec_packet *pkt);


int avcodec_resolve_codecid(const char *s);

/* SDP */
// bool h264_fmtp_cmp(const char *fmtp1, const char *fmtp2, void *data);
// int h264_fmtp_enc(struct mbuf *mb, const struct sdp_format *fmt,
// 		bool offer, void *arg);


/*
 * SDP
 */
uint32_t h264_packetization_mode(const char *fmtp);
int avcodec_h264_fmtp_enc(struct mbuf *mb, const struct sdp_format *fmt,
		  bool offer, void *arg);
bool avcodec_h264_fmtp_cmp(const char *lfmtp, const char *rfmtp, void *data);


//update h264
int vidsrc_init(void);
int visrc_mem_deref(void);
int vidisp_init(void);
int vidisp_mem_deref(void);
int h264_packet_send(struct vtx *vtx, void *buf, int len);


