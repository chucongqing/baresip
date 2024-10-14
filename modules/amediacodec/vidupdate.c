/**
 * @file vidupdate.c update video
 */
#include <unistd.h>
#include <pthread.h>
#include <re.h>
#include <rem.h>
#include <baresip.h>
#include <android/log.h>

struct vidsrc_st {
	const struct vidsrc *vs;  /* inheritance */
	struct vidframe *frame;
	pthread_t thread;
	bool run;
	int fps;
	vidsrc_frame_h *frameh;
	void *arg;
	struct vidsz size;
	pthread_mutex_t mutex;
};

struct vidisp_st {
	int dummy;
};

static struct vidsrc *vidsrc;
static struct vidsrc_st *g_vidsrc_st;

static struct vidisp *vidisp;

void my_vidsrc_update_h264(void *buf, int w, int h, int len, bool is_key)
{
	struct vidframe f;
	struct vidsz size;
	struct vtx *vtx = NULL;

	if (!g_vidsrc_st)
	{
		return;
	}
	pthread_mutex_lock(&(g_vidsrc_st->mutex));
	if (!g_vidsrc_st)
	{
		pthread_mutex_unlock(&(g_vidsrc_st->mutex));
		return;
	}
	
	h264_packet_send(g_vidsrc_st->arg, buf, len);

	pthread_mutex_unlock(&(g_vidsrc_st->mutex));
}

static void src_destructor(void *arg)
{
	struct vidsrc_st *st = arg;

	pthread_mutex_lock(&(st->mutex));

	g_vidsrc_st = NULL;

	if (st->run) {
		st->run = false;
		pthread_join(st->thread, NULL);
	}

	mem_deref(st->frame);

	pthread_mutex_unlock(&(st->mutex));
	pthread_mutex_destroy(&(st->mutex));
}

static int src_alloc(struct vidsrc_st **stp, const struct vidsrc *vs,
		     struct vidsrc_prm *prm,
		     const struct vidsz *size, const char *fmt,
		     const char *dev, vidsrc_frame_h *frameh,
		     vidsrc_packet_h *packeth,
		     vidsrc_error_h *errorh, void *arg)
{
	struct vidsrc_st *st;
	int err;

	(void)fmt;
	(void)dev;
	(void)packeth;
	(void)errorh;

	if (!stp || !prm || !size || !frameh)
		return EINVAL;

	st = mem_zalloc(sizeof(*st), src_destructor);
	if (!st)
		return ENOMEM;

	g_vidsrc_st = st;

	st->vs     = vs;
	st->fps    = prm->fps;
	st->frameh = frameh;
	st->arg    = arg;
	st->size   = *size;

	pthread_mutex_init(&st->mutex, NULL);

	err = vidframe_alloc(&st->frame, VID_FMT_YUV420P, size);
	if (err)
		goto out;

	st->run = true;
	if (err) {
		st->run = false;
		goto out;
	}

 out:
	if (err)
		mem_deref(st);
	else
		*stp = st;

	return err;
}


static void disp_destructor(void *arg)
{
	struct vidisp_st *st = arg;
	(void)st;
}

static int disp_alloc(struct vidisp_st **stp, const struct vidisp *vd,
		      struct vidisp_prm *prm, const char *dev,
		      vidisp_resize_h *resizeh, void *arg)
{
	struct vidisp_st *st;
	(void)prm;
	(void)dev;
	(void)resizeh;
	(void)arg;

	if (!stp || !vd)
		return EINVAL;

	st = mem_zalloc(sizeof(*st), disp_destructor);
	if (!st)
		return ENOMEM;

	*stp = st;

	return 0;

}

static int display(struct vidisp_st *st, const char *title,
		   const struct vidframe *frame, uint64_t timestamp)
{
	(void)st;
	(void)title;
	(void)frame;
	(void)timestamp;

	return 0;
}

int vidsrc_init(void)
{
	int err = 0;
	err |= vidsrc_register(&vidsrc, baresip_vidsrcl(),
			       "vidupdate", src_alloc, NULL);
	return err;
}


int visrc_mem_deref(void)
{
	vidsrc = mem_deref(vidsrc);
	return 0;
}

int vidisp_init(void)
{
	int err = 0;
	err |= vidisp_register(&vidisp, baresip_vidispl(),
					"vidupdate", disp_alloc, NULL, display, NULL);
}

int vidisp_mem_deref(void)
{
	vidisp = mem_deref(vidisp);
	return 0;
}
