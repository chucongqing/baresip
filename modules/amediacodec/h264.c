#include <re.h>
#include <rem.h>
#include <baresip.h>
#include "h264.h"

static struct h264_vidcodec h264 = {
	.vc = {
		.name      = "h264",
		.variant   = "packetization-mode=0",
		.encupdh   = h264_encode_update,
#ifdef H264_SOFT_ENCODE
		.ench      = h264_encode,
#endif
		.decupdh   = h264_decode_update,
		.dech      = h264_decode_h264,
		.fmtp_ench = avcodec_h264_fmtp_enc,
	},
};


static int module_init(void)
{
	vidsrc_init();
	vidisp_init();
	vidcodec_register(baresip_vidcodecl(), (struct vidcodec *)&h264);

	return 0;
}


static int module_close(void)
{
	visrc_mem_deref();
	vidisp_mem_deref();
	vidcodec_unregister((struct vidcodec *)&h264);

	return 0;
}

void load_h264_codec() {
	vidcodec_register(baresip_vidcodecl(), (struct vidcodec *)&h264);
}

void unload_h264_codec() {
	vidcodec_unregister((struct vidcodec *)&h264);
}

EXPORT_SYM const struct mod_export DECL_EXPORTS(jdkcodec) = {
	"amediacodec",
	"codec",
	module_init,
	module_close
};
