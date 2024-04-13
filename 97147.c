static int fallocate_chunk(struct inode *inode, loff_t offset, loff_t len,
			   int mode)
{
	struct gfs2_inode *ip = GFS2_I(inode);
	struct buffer_head *dibh;
	int error;
	loff_t size = len;
	unsigned int nr_blks;
	sector_t lblock = offset >> inode->i_blkbits;

	error = gfs2_meta_inode_buffer(ip, &dibh);
	if (unlikely(error))
		return error;

	gfs2_trans_add_meta(ip->i_gl, dibh);

	if (gfs2_is_stuffed(ip)) {
		error = gfs2_unstuff_dinode(ip, NULL);
		if (unlikely(error))
			goto out;
	}

	while (len) {
		struct buffer_head bh_map = { .b_state = 0, .b_blocknr = 0 };
		bh_map.b_size = len;
		set_buffer_zeronew(&bh_map);

		error = gfs2_block_map(inode, lblock, &bh_map, 1);
		if (unlikely(error))
			goto out;
		len -= bh_map.b_size;
		nr_blks = bh_map.b_size >> inode->i_blkbits;
		lblock += nr_blks;
		if (!buffer_new(&bh_map))
			continue;
		if (unlikely(!buffer_zeronew(&bh_map))) {
			error = -EIO;
			goto out;
		}
	}
	if (offset + size > inode->i_size && !(mode & FALLOC_FL_KEEP_SIZE))
		i_size_write(inode, offset + size);

	mark_inode_dirty(inode);

out:
	brelse(dibh);
	return error;
}
