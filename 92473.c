int drm_mode_dirtyfb_ioctl(struct drm_device *dev,
			   void *data, struct drm_file *file_priv)
{
	struct drm_clip_rect __user *clips_ptr;
	struct drm_clip_rect *clips = NULL;
	struct drm_mode_fb_dirty_cmd *r = data;
	struct drm_mode_object *obj;
	struct drm_framebuffer *fb;
	unsigned flags;
	int num_clips;
	int ret = 0;

	if (!drm_core_check_feature(dev, DRIVER_MODESET))
		return -EINVAL;

	mutex_lock(&dev->mode_config.mutex);
	obj = drm_mode_object_find(dev, r->fb_id, DRM_MODE_OBJECT_FB);
	if (!obj) {
		DRM_ERROR("invalid framebuffer id\n");
		ret = -EINVAL;
		goto out_err1;
	}
	fb = obj_to_fb(obj);

	num_clips = r->num_clips;
	clips_ptr = (struct drm_clip_rect *)(unsigned long)r->clips_ptr;

	if (!num_clips != !clips_ptr) {
		ret = -EINVAL;
		goto out_err1;
	}

	flags = DRM_MODE_FB_DIRTY_FLAGS & r->flags;

	/* If userspace annotates copy, clips must come in pairs */
	if (flags & DRM_MODE_FB_DIRTY_ANNOTATE_COPY && (num_clips % 2)) {
		ret = -EINVAL;
		goto out_err1;
 	}
 
 	if (num_clips && clips_ptr) {
		if (num_clips < 0 || num_clips > DRM_MODE_FB_DIRTY_MAX_CLIPS) {
			ret = -EINVAL;
			goto out_err1;
		}
 		clips = kzalloc(num_clips * sizeof(*clips), GFP_KERNEL);
 		if (!clips) {
 			ret = -ENOMEM;
			goto out_err1;
		}

		ret = copy_from_user(clips, clips_ptr,
				     num_clips * sizeof(*clips));
		if (ret) {
			ret = -EFAULT;
			goto out_err2;
		}
	}

	if (fb->funcs->dirty) {
		ret = fb->funcs->dirty(fb, file_priv, flags, r->color,
				       clips, num_clips);
	} else {
		ret = -ENOSYS;
		goto out_err2;
	}

out_err2:
	kfree(clips);
out_err1:
	mutex_unlock(&dev->mode_config.mutex);
	return ret;
}
