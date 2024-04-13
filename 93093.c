static int walk_hugetlb_range(unsigned long addr, unsigned long end,
			      struct mm_walk *walk)
{
	struct vm_area_struct *vma = walk->vma;
	struct hstate *h = hstate_vma(vma);
	unsigned long next;
	unsigned long hmask = huge_page_mask(h);
	unsigned long sz = huge_page_size(h);
	pte_t *pte;
	int err = 0;

 	do {
 		next = hugetlb_entry_end(h, addr, end);
 		pte = huge_pte_offset(walk->mm, addr & hmask, sz);

		if (pte)
 			err = walk->hugetlb_entry(pte, hmask, addr, next, walk);
		else if (walk->pte_hole)
			err = walk->pte_hole(addr, next, walk);

 		if (err)
 			break;
 	} while (addr = next, addr != end);

	return err;
}
