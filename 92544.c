static int uio_mmap_physical(struct vm_area_struct *vma)
 {
 	struct uio_device *idev = vma->vm_private_data;
 	int mi = uio_find_mem_index(vma);
	struct uio_mem *mem;
 	if (mi < 0)
 		return -EINVAL;
	mem = idev->info->mem + mi;
 
	if (vma->vm_end - vma->vm_start > mem->size)
		return -EINVAL;
 
	vma->vm_ops = &uio_physical_vm_ops;
 	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
 
	/*
	 * We cannot use the vm_iomap_memory() helper here,
	 * because vma->vm_pgoff is the map index we looked
	 * up above in uio_find_mem_index(), rather than an
	 * actual page offset into the mmap.
	 *
	 * So we just do the physical mmap without a page
	 * offset.
	 */
 	return remap_pfn_range(vma,
 			       vma->vm_start,
			       mem->addr >> PAGE_SHIFT,
 			       vma->vm_end - vma->vm_start,
 			       vma->vm_page_prot);
 }
