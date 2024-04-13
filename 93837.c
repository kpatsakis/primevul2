int hugetlb_mcopy_atomic_pte(struct mm_struct *dst_mm,
			    pte_t *dst_pte,
			    struct vm_area_struct *dst_vma,
			    unsigned long dst_addr,
 			    unsigned long src_addr,
 			    struct page **pagep)
 {
	struct address_space *mapping;
	pgoff_t idx;
	unsigned long size;
 	int vm_shared = dst_vma->vm_flags & VM_SHARED;
 	struct hstate *h = hstate_vma(dst_vma);
 	pte_t _dst_pte;
	spinlock_t *ptl;
	int ret;
	struct page *page;

	if (!*pagep) {
		ret = -ENOMEM;
		page = alloc_huge_page(dst_vma, dst_addr, 0);
		if (IS_ERR(page))
			goto out;

		ret = copy_huge_page_from_user(page,
						(const void __user *) src_addr,
						pages_per_huge_page(h), false);

		/* fallback to copy_from_user outside mmap_sem */
		if (unlikely(ret)) {
			ret = -EFAULT;
			*pagep = page;
			/* don't free the page */
			goto out;
		}
	} else {
		page = *pagep;
		*pagep = NULL;
	}

	/*
	 * The memory barrier inside __SetPageUptodate makes sure that
	 * preceding stores to the page contents become visible before
	 * the set_pte_at() write.
	 */
 	__SetPageUptodate(page);
 	set_page_huge_active(page);
 
	mapping = dst_vma->vm_file->f_mapping;
	idx = vma_hugecache_offset(h, dst_vma, dst_addr);

 	/*
 	 * If shared, add to page cache
 	 */
 	if (vm_shared) {
		size = i_size_read(mapping->host) >> huge_page_shift(h);
		ret = -EFAULT;
		if (idx >= size)
			goto out_release_nounlock;
 
		/*
		 * Serialization between remove_inode_hugepages() and
		 * huge_add_to_page_cache() below happens through the
		 * hugetlb_fault_mutex_table that here must be hold by
		 * the caller.
		 */
 		ret = huge_add_to_page_cache(page, mapping, idx);
 		if (ret)
 			goto out_release_nounlock;
	}

 	ptl = huge_pte_lockptr(h, dst_mm, dst_pte);
 	spin_lock(ptl);
 
	/*
	 * Recheck the i_size after holding PT lock to make sure not
	 * to leave any page mapped (as page_mapped()) beyond the end
	 * of the i_size (remove_inode_hugepages() is strict about
	 * enforcing that). If we bail out here, we'll also leave a
	 * page in the radix tree in the vm_shared case beyond the end
	 * of the i_size, but remove_inode_hugepages() will take care
	 * of it as soon as we drop the hugetlb_fault_mutex_table.
	 */
	size = i_size_read(mapping->host) >> huge_page_shift(h);
	ret = -EFAULT;
	if (idx >= size)
		goto out_release_unlock;

 	ret = -EEXIST;
 	if (!huge_pte_none(huge_ptep_get(dst_pte)))
 		goto out_release_unlock;

	if (vm_shared) {
		page_dup_rmap(page, true);
	} else {
		ClearPagePrivate(page);
		hugepage_add_new_anon_rmap(page, dst_vma, dst_addr);
	}

	_dst_pte = make_huge_pte(dst_vma, page, dst_vma->vm_flags & VM_WRITE);
	if (dst_vma->vm_flags & VM_WRITE)
		_dst_pte = huge_pte_mkdirty(_dst_pte);
	_dst_pte = pte_mkyoung(_dst_pte);

	set_huge_pte_at(dst_mm, dst_addr, dst_pte, _dst_pte);

	(void)huge_ptep_set_access_flags(dst_vma, dst_addr, dst_pte, _dst_pte,
					dst_vma->vm_flags & VM_WRITE);
	hugetlb_count_add(pages_per_huge_page(h), dst_mm);

	/* No need to invalidate - it was non-present before */
	update_mmu_cache(dst_vma, dst_addr, dst_pte);

	spin_unlock(ptl);
	if (vm_shared)
		unlock_page(page);
	ret = 0;
out:
	return ret;
out_release_unlock:
	spin_unlock(ptl);
	if (vm_shared)
		unlock_page(page);
out_release_nounlock:
	put_page(page);
	goto out;
}
