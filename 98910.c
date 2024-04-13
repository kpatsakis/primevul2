hfs_inode_walk(TSK_FS_INFO * fs, TSK_INUM_T start_inum,
    TSK_INUM_T end_inum, TSK_FS_META_FLAG_ENUM flags,
    TSK_FS_META_WALK_CB action, void *ptr)
{
    TSK_INUM_T inum;
    TSK_FS_FILE *fs_file;

    if (tsk_verbose)
        tsk_fprintf(stderr,
            "hfs_inode_walk: start_inum: %" PRIuINUM " end_inum: %"
            PRIuINUM " flags: %" PRIu32 "\n", start_inum, end_inum, flags);

    /*
     * Sanity checks.
     */
    if (start_inum < fs->first_inum || start_inum > fs->last_inum) {
        tsk_error_reset();
        tsk_error_set_errno(TSK_ERR_FS_WALK_RNG);
        tsk_error_set_errstr("inode_walk: Start inode: %" PRIuINUM "",
            start_inum);
        return 1;
    }
    else if (end_inum < fs->first_inum || end_inum > fs->last_inum
        || end_inum < start_inum) {
        tsk_error_reset();
        tsk_error_set_errno(TSK_ERR_FS_WALK_RNG);
        tsk_error_set_errstr("inode_walk: End inode: %" PRIuINUM "",
            end_inum);
        return 1;
    }

    /* If ORPHAN is wanted, then make sure that the flags are correct */
    if (flags & TSK_FS_META_FLAG_ORPHAN) {
        flags |= TSK_FS_META_FLAG_UNALLOC;
        flags &= ~TSK_FS_META_FLAG_ALLOC;
        flags |= TSK_FS_META_FLAG_USED;
        flags &= ~TSK_FS_META_FLAG_UNUSED;
    }

    else {
        if (((flags & TSK_FS_META_FLAG_ALLOC) == 0) &&
            ((flags & TSK_FS_META_FLAG_UNALLOC) == 0)) {
            flags |= (TSK_FS_META_FLAG_ALLOC | TSK_FS_META_FLAG_UNALLOC);
        }

        /* If neither of the USED or UNUSED flags are set, then set them
         * both
         */
        if (((flags & TSK_FS_META_FLAG_USED) == 0) &&
            ((flags & TSK_FS_META_FLAG_UNUSED) == 0)) {
            flags |= (TSK_FS_META_FLAG_USED | TSK_FS_META_FLAG_UNUSED);
        }
    }

    if ((fs_file = tsk_fs_file_alloc(fs)) == NULL)
        return 1;

    if ((fs_file->meta = tsk_fs_meta_alloc(HFS_FILE_CONTENT_LEN)) == NULL)
        return 1;

    if (start_inum > end_inum)
        XSWAP(start_inum, end_inum);

    for (inum = start_inum; inum <= end_inum; ++inum) {
        int retval;

        if (hfs_inode_lookup(fs, fs_file, inum)) {
            if (tsk_error_get_errno() == TSK_ERR_FS_INODE_NUM) {
                tsk_error_reset();
                continue;
            }
            else {
                return 1;
            }
        }

        if ((fs_file->meta->flags & flags) != fs_file->meta->flags)
            continue;

        /* call action */
        retval = action(fs_file, ptr);
        if (retval == TSK_WALK_STOP) {
            tsk_fs_file_close(fs_file);
            return 0;
        }
        else if (retval == TSK_WALK_ERROR) {
            tsk_fs_file_close(fs_file);
            return 1;
        }
    }

    tsk_fs_file_close(fs_file);
    return 0;
}
