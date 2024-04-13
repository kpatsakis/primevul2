struct dentry *debugfs_rename(struct dentry *old_dir, struct dentry *old_dentry,
		struct dentry *new_dir, const char *new_name)
 {
 	int error;
 	struct dentry *dentry = NULL, *trap;
	struct name_snapshot old_name;
 
 	trap = lock_rename(new_dir, old_dir);
 	/* Source or destination directories don't exist? */
	if (d_really_is_negative(old_dir) || d_really_is_negative(new_dir))
		goto exit;
	/* Source does not exist, cyclic rename, or mountpoint? */
	if (d_really_is_negative(old_dentry) || old_dentry == trap ||
	    d_mountpoint(old_dentry))
		goto exit;
	dentry = lookup_one_len(new_name, new_dir, strlen(new_name));
	/* Lookup failed, cyclic rename or target exists? */
 	if (IS_ERR(dentry) || dentry == trap || d_really_is_positive(dentry))
 		goto exit;
 
	take_dentry_name_snapshot(&old_name, old_dentry);
 
 	error = simple_rename(d_inode(old_dir), old_dentry, d_inode(new_dir),
 			      dentry, 0);
 	if (error) {
		release_dentry_name_snapshot(&old_name);
 		goto exit;
 	}
 	d_move(old_dentry, dentry);
	fsnotify_move(d_inode(old_dir), d_inode(new_dir), old_name.name,
 		d_is_dir(old_dentry),
 		NULL, old_dentry);
	release_dentry_name_snapshot(&old_name);
 	unlock_rename(new_dir, old_dir);
 	dput(dentry);
 	return old_dentry;
exit:
	if (dentry && !IS_ERR(dentry))
		dput(dentry);
	unlock_rename(new_dir, old_dir);
	return NULL;
}