struct mount *__lookup_mnt(struct vfsmount *mnt, struct dentry *dentry,
			      int dir)
{
	struct list_head *head = mount_hashtable + hash(mnt, dentry);
	struct list_head *tmp = head;
	struct mount *p, *found = NULL;

	for (;;) {
		tmp = dir ? tmp->next : tmp->prev;
		p = NULL;
		if (tmp == head)
			break;
		p = list_entry(tmp, struct mount, mnt_hash);
		if (&p->mnt_parent->mnt == mnt && p->mnt_mountpoint == dentry) {
			found = p;
			break;
		}
	}
	return found;
}
