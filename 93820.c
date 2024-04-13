 static bool add_free_nid(struct f2fs_sb_info *sbi, nid_t nid, bool build)
 {
 	struct f2fs_nm_info *nm_i = NM_I(sbi);
	struct free_nid *i, *e;
 	struct nat_entry *ne;
	int err = -EINVAL;
	bool ret = false;
 
 	/* 0 nid should not be used */
 	if (unlikely(nid == 0))
 		return false;
 
 	i = f2fs_kmem_cache_alloc(free_nid_slab, GFP_NOFS);
 	i->nid = nid;
 	i->state = NID_NEW;
 
	if (radix_tree_preload(GFP_NOFS))
		goto err;
 
 	spin_lock(&nm_i->nid_list_lock);

	if (build) {
		/*
		 *   Thread A             Thread B
		 *  - f2fs_create
		 *   - f2fs_new_inode
		 *    - alloc_nid
		 *     - __insert_nid_to_list(ALLOC_NID_LIST)
		 *                     - f2fs_balance_fs_bg
		 *                      - build_free_nids
		 *                       - __build_free_nids
		 *                        - scan_nat_page
		 *                         - add_free_nid
		 *                          - __lookup_nat_cache
		 *  - f2fs_add_link
		 *   - init_inode_metadata
		 *    - new_inode_page
		 *     - new_node_page
		 *      - set_node_addr
		 *  - alloc_nid_done
		 *   - __remove_nid_from_list(ALLOC_NID_LIST)
		 *                         - __insert_nid_to_list(FREE_NID_LIST)
		 */
		ne = __lookup_nat_cache(nm_i, nid);
		if (ne && (!get_nat_flag(ne, IS_CHECKPOINTED) ||
				nat_get_blkaddr(ne) != NULL_ADDR))
			goto err_out;

		e = __lookup_free_nid_list(nm_i, nid);
		if (e) {
			if (e->state == NID_NEW)
				ret = true;
			goto err_out;
		}
	}
	ret = true;
 	err = __insert_nid_to_list(sbi, i, FREE_NID_LIST, true);
err_out:
 	spin_unlock(&nm_i->nid_list_lock);
 	radix_tree_preload_end();
err:
	if (err)
 		kmem_cache_free(free_nid_slab, i);
	return ret;
 }
