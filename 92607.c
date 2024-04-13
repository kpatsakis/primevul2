fb_mmap(struct file *file, struct vm_area_struct * vma)
 {
 	struct fb_info *info = file_fb_info(file);
 	struct fb_ops *fb;
	unsigned long mmio_pgoff;
 	unsigned long start;
 	u32 len;
 
 	if (!info)
 		return -ENODEV;
 	fb = info->fbops;
 	if (!fb)
 		return -ENODEV;
	mutex_lock(&info->mm_lock);
	if (fb->fb_mmap) {
		int res;
		res = fb->fb_mmap(info, vma);
		mutex_unlock(&info->mm_lock);
 		return res;
 	}
 
	/*
	 * Ugh. This can be either the frame buffer mapping, or
	 * if pgoff points past it, the mmio mapping.
	 */
 	start = info->fix.smem_start;
	len = info->fix.smem_len;
	mmio_pgoff = PAGE_ALIGN((start & ~PAGE_MASK) + len) >> PAGE_SHIFT;
	if (vma->vm_pgoff >= mmio_pgoff) {
		vma->vm_pgoff -= mmio_pgoff;
 		start = info->fix.mmio_start;
		len = info->fix.mmio_len;
 	}
 	mutex_unlock(&info->mm_lock);

 	vma->vm_page_prot = vm_get_page_prot(vma->vm_flags);
	fb_pgprotect(file, vma, start);

	return vm_iomap_memory(vma, start, len);
 }