static int udf_do_extend_file(struct inode *inode,
			      struct extent_position *last_pos,
			      struct kernel_long_ad *last_ext,
			      sector_t blocks)
{
	sector_t add;
	int count = 0, fake = !(last_ext->extLength & UDF_EXTENT_LENGTH_MASK);
	struct super_block *sb = inode->i_sb;
	struct kernel_lb_addr prealloc_loc = {};
	int prealloc_len = 0;
	struct udf_inode_info *iinfo;
	int err;

	/* The previous extent is fake and we should not extend by anything
	 * - there's nothing to do... */
	if (!blocks && fake)
		return 0;

	iinfo = UDF_I(inode);
	/* Round the last extent up to a multiple of block size */
	if (last_ext->extLength & (sb->s_blocksize - 1)) {
		last_ext->extLength =
			(last_ext->extLength & UDF_EXTENT_FLAG_MASK) |
			(((last_ext->extLength & UDF_EXTENT_LENGTH_MASK) +
			  sb->s_blocksize - 1) & ~(sb->s_blocksize - 1));
		iinfo->i_lenExtents =
			(iinfo->i_lenExtents + sb->s_blocksize - 1) &
			~(sb->s_blocksize - 1);
	}

	/* Last extent are just preallocated blocks? */
	if ((last_ext->extLength & UDF_EXTENT_FLAG_MASK) ==
						EXT_NOT_RECORDED_ALLOCATED) {
		/* Save the extent so that we can reattach it to the end */
		prealloc_loc = last_ext->extLocation;
		prealloc_len = last_ext->extLength;
		/* Mark the extent as a hole */
		last_ext->extLength = EXT_NOT_RECORDED_NOT_ALLOCATED |
			(last_ext->extLength & UDF_EXTENT_LENGTH_MASK);
		last_ext->extLocation.logicalBlockNum = 0;
		last_ext->extLocation.partitionReferenceNum = 0;
	}

	/* Can we merge with the previous extent? */
	if ((last_ext->extLength & UDF_EXTENT_FLAG_MASK) ==
					EXT_NOT_RECORDED_NOT_ALLOCATED) {
		add = ((1 << 30) - sb->s_blocksize -
			(last_ext->extLength & UDF_EXTENT_LENGTH_MASK)) >>
			sb->s_blocksize_bits;
		if (add > blocks)
			add = blocks;
		blocks -= add;
		last_ext->extLength += add << sb->s_blocksize_bits;
	}

	if (fake) {
		udf_add_aext(inode, last_pos, &last_ext->extLocation,
			     last_ext->extLength, 1);
		count++;
	} else
		udf_write_aext(inode, last_pos, &last_ext->extLocation,
				last_ext->extLength, 1);

	/* Managed to do everything necessary? */
	if (!blocks)
		goto out;

	/* All further extents will be NOT_RECORDED_NOT_ALLOCATED */
	last_ext->extLocation.logicalBlockNum = 0;
	last_ext->extLocation.partitionReferenceNum = 0;
	add = (1 << (30-sb->s_blocksize_bits)) - 1;
	last_ext->extLength = EXT_NOT_RECORDED_NOT_ALLOCATED |
				(add << sb->s_blocksize_bits);

	/* Create enough extents to cover the whole hole */
	while (blocks > add) {
		blocks -= add;
		err = udf_add_aext(inode, last_pos, &last_ext->extLocation,
				   last_ext->extLength, 1);
		if (err)
			return err;
		count++;
	}
	if (blocks) {
		last_ext->extLength = EXT_NOT_RECORDED_NOT_ALLOCATED |
			(blocks << sb->s_blocksize_bits);
		err = udf_add_aext(inode, last_pos, &last_ext->extLocation,
				   last_ext->extLength, 1);
		if (err)
			return err;
		count++;
	}

out:
	/* Do we have some preallocated blocks saved? */
	if (prealloc_len) {
		err = udf_add_aext(inode, last_pos, &prealloc_loc,
				   prealloc_len, 1);
		if (err)
			return err;
		last_ext->extLocation = prealloc_loc;
		last_ext->extLength = prealloc_len;
		count++;
	}

	/* last_pos should point to the last written extent... */
	if (iinfo->i_alloc_type == ICBTAG_FLAG_AD_SHORT)
		last_pos->offset -= sizeof(struct short_ad);
	else if (iinfo->i_alloc_type == ICBTAG_FLAG_AD_LONG)
		last_pos->offset -= sizeof(struct long_ad);
	else
		return -EIO;

	return count;
}
