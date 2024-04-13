 int fscrypt_process_policy(struct inode *inode,
 				const struct fscrypt_policy *policy)
 {
	if (!inode_owner_or_capable(inode))
		return -EACCES;

 	if (policy->version != 0)
 		return -EINVAL;
 
	if (!inode_has_encryption_context(inode)) {
		if (!inode->i_sb->s_cop->empty_dir)
			return -EOPNOTSUPP;
		if (!inode->i_sb->s_cop->empty_dir(inode))
			return -ENOTEMPTY;
		return create_encryption_context_from_policy(inode, policy);
	}

	if (is_encryption_context_consistent_with_policy(inode, policy))
		return 0;

	printk(KERN_WARNING "%s: Policy inconsistent with encryption context\n",
	       __func__);
	return -EINVAL;
}
