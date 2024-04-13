int simple_set_acl(struct inode *inode, struct posix_acl *acl, int type)
{
 	int error;
 
 	if (type == ACL_TYPE_ACCESS) {
		error = posix_acl_update_mode(inode,
				&inode->i_mode, &acl);
		if (error)
			return error;
 	}
 
 	inode->i_ctime = current_time(inode);
	set_cached_acl(inode, type, acl);
	return 0;
}
