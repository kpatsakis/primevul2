static struct dentry *aio_mount(struct file_system_type *fs_type,
				int flags, const char *dev_name, void *data)
{
 	static const struct dentry_operations ops = {
 		.d_dname	= simple_dname,
 	};
	struct dentry *root = mount_pseudo(fs_type, "aio:", NULL, &ops,
					   AIO_RING_MAGIC);

	if (!IS_ERR(root))
		root->d_sb->s_iflags |= SB_I_NOEXEC;
	return root;
 }
