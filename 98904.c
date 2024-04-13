hfs_block_walk(TSK_FS_INFO * fs, TSK_DADDR_T start_blk,
    TSK_DADDR_T end_blk, TSK_FS_BLOCK_WALK_FLAG_ENUM flags,
    TSK_FS_BLOCK_WALK_CB action, void *ptr)
{
    char *myname = "hfs_block_walk";
    HFS_INFO *hfs = (HFS_INFO *) fs;
    TSK_FS_BLOCK *fs_block;
    TSK_DADDR_T addr;

    if (tsk_verbose)
        tsk_fprintf(stderr,
            "%s: start_blk: %" PRIuDADDR " end_blk: %"
            PRIuDADDR " flags: %" PRIu32 "\n", myname, start_blk, end_blk,
            flags);

    tsk_error_reset();

    /*
     * Sanity checks.
     */
    if (start_blk < fs->first_block || start_blk > fs->last_block) {
        tsk_error_set_errno(TSK_ERR_FS_WALK_RNG);
        tsk_error_set_errstr("%s: invalid start block number: %" PRIuDADDR
            "", myname, start_blk);
        return 1;
    }
    if (end_blk < fs->first_block || end_blk > fs->last_block) {
        tsk_error_set_errno(TSK_ERR_FS_WALK_RNG);
        tsk_error_set_errstr("%s: invalid last block number: %" PRIuDADDR
            "", myname, end_blk);
        return 1;
    }

    if (start_blk > end_blk)
        XSWAP(start_blk, end_blk);

    /* Sanity check on flags -- make sure at least one ALLOC is set */
    if (((flags & TSK_FS_BLOCK_WALK_FLAG_ALLOC) == 0) &&
        ((flags & TSK_FS_BLOCK_WALK_FLAG_UNALLOC) == 0)) {
        flags |=
            (TSK_FS_BLOCK_WALK_FLAG_ALLOC |
            TSK_FS_BLOCK_WALK_FLAG_UNALLOC);
    }
    if (((flags & TSK_FS_BLOCK_WALK_FLAG_META) == 0) &&
        ((flags & TSK_FS_BLOCK_WALK_FLAG_CONT) == 0)) {
        flags |=
            (TSK_FS_BLOCK_WALK_FLAG_CONT | TSK_FS_BLOCK_WALK_FLAG_META);
    }

    if ((fs_block = tsk_fs_block_alloc(fs)) == NULL) {
        return 1;
    }

    /*
     * Iterate
     */
    for (addr = start_blk; addr <= end_blk; ++addr) {
        int retval;
        int myflags;

        /* identify if the block is allocated or not */
        myflags = hfs_block_is_alloc(hfs, addr) ?
            TSK_FS_BLOCK_FLAG_ALLOC : TSK_FS_BLOCK_FLAG_UNALLOC;

        if ((myflags & TSK_FS_BLOCK_FLAG_ALLOC)
            && (!(flags & TSK_FS_BLOCK_WALK_FLAG_ALLOC)))
            continue;
        else if ((myflags & TSK_FS_BLOCK_FLAG_UNALLOC)
            && (!(flags & TSK_FS_BLOCK_WALK_FLAG_UNALLOC)))
            continue;

        if (flags & TSK_FS_BLOCK_WALK_FLAG_AONLY)
            myflags |= TSK_FS_BLOCK_FLAG_AONLY;

        if (tsk_fs_block_get_flag(fs, fs_block, addr,
                (TSK_FS_BLOCK_FLAG_ENUM) myflags) == NULL) {
            tsk_fs_block_free(fs_block);
            return 1;
        }

        retval = action(fs_block, ptr);
        if (TSK_WALK_STOP == retval) {
            break;
        }
        else if (TSK_WALK_ERROR == retval) {
            tsk_fs_block_free(fs_block);
            return 1;
        }
    }

    tsk_fs_block_free(fs_block);
    return 0;
}
