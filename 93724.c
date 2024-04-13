 xfs_dinode_verify(
 	struct xfs_mount	*mp,
	xfs_ino_t		ino,
	struct xfs_dinode	*dip)
{
	xfs_failaddr_t		fa;
	uint16_t		mode;
	uint16_t		flags;
	uint64_t		flags2;
	uint64_t		di_size;

	if (dip->di_magic != cpu_to_be16(XFS_DINODE_MAGIC))
		return __this_address;

	/* Verify v3 integrity information first */
	if (dip->di_version >= 3) {
		if (!xfs_sb_version_hascrc(&mp->m_sb))
			return __this_address;
		if (!xfs_verify_cksum((char *)dip, mp->m_sb.sb_inodesize,
				      XFS_DINODE_CRC_OFF))
			return __this_address;
		if (be64_to_cpu(dip->di_ino) != ino)
			return __this_address;
		if (!uuid_equal(&dip->di_uuid, &mp->m_sb.sb_meta_uuid))
			return __this_address;
	}

	/* don't allow invalid i_size */
	di_size = be64_to_cpu(dip->di_size);
	if (di_size & (1ULL << 63))
		return __this_address;

	mode = be16_to_cpu(dip->di_mode);
	if (mode && xfs_mode_to_ftype(mode) == XFS_DIR3_FT_UNKNOWN)
		return __this_address;

	/* No zero-length symlinks/dirs. */
	if ((S_ISLNK(mode) || S_ISDIR(mode)) && di_size == 0)
		return __this_address;

	/* Fork checks carried over from xfs_iformat_fork */
	if (mode &&
	    be32_to_cpu(dip->di_nextents) + be16_to_cpu(dip->di_anextents) >
			be64_to_cpu(dip->di_nblocks))
		return __this_address;

	if (mode && XFS_DFORK_BOFF(dip) > mp->m_sb.sb_inodesize)
		return __this_address;

	flags = be16_to_cpu(dip->di_flags);

	if (mode && (flags & XFS_DIFLAG_REALTIME) && !mp->m_rtdev_targp)
		return __this_address;

	/* Do we have appropriate data fork formats for the mode? */
	switch (mode & S_IFMT) {
	case S_IFIFO:
	case S_IFCHR:
	case S_IFBLK:
	case S_IFSOCK:
		if (dip->di_format != XFS_DINODE_FMT_DEV)
			return __this_address;
		break;
 	case S_IFREG:
 	case S_IFLNK:
 	case S_IFDIR:
		fa = xfs_dinode_verify_fork(dip, mp, XFS_DATA_FORK);
		if (fa)
			return fa;
 		break;
 	case 0:
 		/* Uninitialized inode ok. */
		break;
	default:
		return __this_address;
 	}
 
 	if (XFS_DFORK_Q(dip)) {
		fa = xfs_dinode_verify_fork(dip, mp, XFS_ATTR_FORK);
		if (fa)
			return fa;
 	} else {
 		/*
 		 * If there is no fork offset, this may be a freshly-made inode
		 * in a new disk cluster, in which case di_aformat is zeroed.
		 * Otherwise, such an inode must be in EXTENTS format; this goes
		 * for freed inodes as well.
		 */
		switch (dip->di_aformat) {
		case 0:
		case XFS_DINODE_FMT_EXTENTS:
			break;
		default:
			return __this_address;
		}
		if (dip->di_anextents)
			return __this_address;
	}

	/* extent size hint validation */
	fa = xfs_inode_validate_extsize(mp, be32_to_cpu(dip->di_extsize),
			mode, flags);
	if (fa)
		return fa;

	/* only version 3 or greater inodes are extensively verified here */
	if (dip->di_version < 3)
		return NULL;

	flags2 = be64_to_cpu(dip->di_flags2);

	/* don't allow reflink/cowextsize if we don't have reflink */
	if ((flags2 & (XFS_DIFLAG2_REFLINK | XFS_DIFLAG2_COWEXTSIZE)) &&
	     !xfs_sb_version_hasreflink(&mp->m_sb))
		return __this_address;

	/* only regular files get reflink */
	if ((flags2 & XFS_DIFLAG2_REFLINK) && (mode & S_IFMT) != S_IFREG)
		return __this_address;

	/* don't let reflink and realtime mix */
	if ((flags2 & XFS_DIFLAG2_REFLINK) && (flags & XFS_DIFLAG_REALTIME))
		return __this_address;

	/* don't let reflink and dax mix */
	if ((flags2 & XFS_DIFLAG2_REFLINK) && (flags2 & XFS_DIFLAG2_DAX))
		return __this_address;

	/* COW extent size hint validation */
	fa = xfs_inode_validate_cowextsize(mp, be32_to_cpu(dip->di_cowextsize),
			mode, flags, flags2);
	if (fa)
		return fa;

	return NULL;
}