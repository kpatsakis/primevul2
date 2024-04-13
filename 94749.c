static uint32_t *nfs_read_post_op_attr(uint32_t *p, struct inode *inode)
{
	/*
	 * union post_op_attr switch (bool attributes_follow) {
	 * case TRUE:
	 * 	fattr3 attributes;
	 * case FALSE:
	 * 	void;
	 * };
	 */

	if (ntoh32(net_read_uint32(p++))) {
		nfs_fattr3_to_stat(p, inode);
		p += 21;
	}

	return p;
}
