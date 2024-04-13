static int kvm_debugfs_open(struct inode *inode, struct file *file,
			   int (*get)(void *, u64 *), int (*set)(void *, u64),
			   const char *fmt)
{
	struct kvm_stat_data *stat_data = (struct kvm_stat_data *)
					  inode->i_private;

	/* The debugfs files are a reference to the kvm struct which
	 * is still valid when kvm_destroy_vm is called.
	 * To avoid the race between open and the removal of the debugfs
	 * directory we test against the users count.
	 */
	if (!refcount_inc_not_zero(&stat_data->kvm->users_count))
		return -ENOENT;

	if (simple_attr_open(inode, file, get, set, fmt)) {
		kvm_put_kvm(stat_data->kvm);
		return -ENOMEM;
	}

	return 0;
}