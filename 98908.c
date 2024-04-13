hfs_find_highest_inum(HFS_INFO * hfs)
{
    TSK_INUM_T inum;
    if (hfs_cat_traverse(hfs, hfs_find_highest_inum_cb, &inum)) {
      /* Catalog traversal failed, fallback on legacy method :
         if HFS_VH_ATTR_CNIDS_REUSED is set, then
         the maximum CNID is 2^32-1; if it's not set, then nextCatalogId is
         supposed to be larger than all CNIDs on disk.
       */
        TSK_FS_INFO *fs = (TSK_FS_INFO *) & (hfs->fs_info);
        if (tsk_getu32(fs->endian, hfs->fs->attr) & HFS_VH_ATTR_CNIDS_REUSED)
            return (TSK_INUM_T) 0xffffffff;
        else
            return (TSK_INUM_T) tsk_getu32(fs->endian,
                hfs->fs->next_cat_id) - 1;
    }
    return inum;
}
