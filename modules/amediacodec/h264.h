struct h264_vidcodec {
	struct vidcodec vc;
};

/* Encode */
int h264_encode_update(struct videnc_state **vesp, const struct vidcodec *vc,
		struct videnc_param *prm, const char *fmtp,
		videnc_packet_h *pkth, void *arg);
int h264_encode(struct videnc_state *ves, bool update,
		const struct vidframe *frame);


/* Decode */
int h264_decode_update(struct viddec_state **vdsp, const struct vidcodec *vc,
		const char *fmtp);
int h264_decode(struct viddec_state *vds, struct vidframe *frame,
		bool *intra, bool marker, uint16_t seq, struct mbuf *mb);


/* SDP */
bool h264_fmtp_cmp(const char *fmtp1, const char *fmtp2, void *data);
int h264_fmtp_enc(struct mbuf *mb, const struct sdp_format *fmt,
		bool offer, void *arg);

//update h264
int vidsrc_init(void);
int visrc_mem_deref(void);
int vidisp_init(void);
int vidisp_mem_deref(void);
int h264_packet_send(struct vtx *vtx, void *buf, int len);


