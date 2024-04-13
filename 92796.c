int ovl_setattr(struct dentry *dentry, struct iattr *attr)
{
	int err;
	struct dentry *upperdentry;

	err = ovl_want_write(dentry);
 	if (err)
 		goto out;
 
	err = ovl_copy_up(dentry);
	if (!err) {
		upperdentry = ovl_dentry_upper(dentry);

 		mutex_lock(&upperdentry->d_inode->i_mutex);
 		err = notify_change(upperdentry, attr, NULL);
 		mutex_unlock(&upperdentry->d_inode->i_mutex);
 	}
 	ovl_drop_write(dentry);
 out:
	return err;
}