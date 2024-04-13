static struct mnt_namespace *dup_mnt_ns(struct mnt_namespace *mnt_ns,
		struct user_namespace *user_ns, struct fs_struct *fs)
{
	struct mnt_namespace *new_ns;
	struct vfsmount *rootmnt = NULL, *pwdmnt = NULL;
	struct mount *p, *q;
	struct mount *old = mnt_ns->root;
	struct mount *new;
	int copy_flags;

	new_ns = alloc_mnt_ns(user_ns);
	if (IS_ERR(new_ns))
		return new_ns;

	down_write(&namespace_sem);
 	/* First pass: copy the tree topology */
 	copy_flags = CL_COPY_ALL | CL_EXPIRE;
 	if (user_ns != mnt_ns->user_ns)
		copy_flags |= CL_SHARED_TO_SLAVE | CL_UNPRIVILEGED;
 	new = copy_tree(old, old->mnt.mnt_root, copy_flags);
 	if (IS_ERR(new)) {
 		up_write(&namespace_sem);
		free_mnt_ns(new_ns);
		return ERR_CAST(new);
	}
	new_ns->root = new;
	br_write_lock(&vfsmount_lock);
	list_add_tail(&new_ns->list, &new->mnt_list);
	br_write_unlock(&vfsmount_lock);

	/*
	 * Second pass: switch the tsk->fs->* elements and mark new vfsmounts
	 * as belonging to new namespace.  We have already acquired a private
	 * fs_struct, so tsk->fs->lock is not needed.
	 */
	p = old;
	q = new;
	while (p) {
		q->mnt_ns = new_ns;
		if (fs) {
			if (&p->mnt == fs->root.mnt) {
				fs->root.mnt = mntget(&q->mnt);
				rootmnt = &p->mnt;
			}
			if (&p->mnt == fs->pwd.mnt) {
				fs->pwd.mnt = mntget(&q->mnt);
				pwdmnt = &p->mnt;
			}
		}
		p = next_mnt(p, old);
		q = next_mnt(q, new);
	}
	up_write(&namespace_sem);

	if (rootmnt)
		mntput(rootmnt);
	if (pwdmnt)
		mntput(pwdmnt);

	return new_ns;
}
