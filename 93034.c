static __be32 nfsd3_proc_setacl(struct svc_rqst * rqstp,
		struct nfsd3_setaclargs *argp,
		struct nfsd3_attrstat *resp)
{
	struct inode *inode;
	svc_fh *fh;
	__be32 nfserr = 0;
	int error;

	fh = fh_copy(&resp->fh, &argp->fh);
	nfserr = fh_verify(rqstp, &resp->fh, 0, NFSD_MAY_SATTR);
	if (nfserr)
 		goto out;
 
 	inode = d_inode(fh->fh_dentry);
 
 	error = fh_want_write(fh);
 	if (error)
 		goto out_errno;
 
	fh_lock(fh);

	error = set_posix_acl(inode, ACL_TYPE_ACCESS, argp->acl_access);
 	if (error)
		goto out_drop_lock;
	error = set_posix_acl(inode, ACL_TYPE_DEFAULT, argp->acl_default);
 
out_drop_lock:
	fh_unlock(fh);
 	fh_drop_write(fh);
 out_errno:
 	nfserr = nfserrno(error);
out:
	/* argp->acl_{access,default} may have been allocated in
	   nfs3svc_decode_setaclargs. */
	posix_acl_release(argp->acl_access);
	posix_acl_release(argp->acl_default);
	RETURN_STATUS(nfserr);
}
