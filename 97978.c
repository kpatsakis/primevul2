void migrate_page_copy(struct page *newpage, struct page *page)
{
	int cpupid;

	if (PageHuge(page) || PageTransHuge(page))
		copy_huge_page(newpage, page);
	else
		copy_highpage(newpage, page);

	if (PageError(page))
		SetPageError(newpage);
	if (PageReferenced(page))
		SetPageReferenced(newpage);
	if (PageUptodate(page))
		SetPageUptodate(newpage);
	if (TestClearPageActive(page)) {
		VM_BUG_ON_PAGE(PageUnevictable(page), page);
		SetPageActive(newpage);
	} else if (TestClearPageUnevictable(page))
		SetPageUnevictable(newpage);
	if (PageChecked(page))
		SetPageChecked(newpage);
	if (PageMappedToDisk(page))
		SetPageMappedToDisk(newpage);

	/* Move dirty on pages not done by migrate_page_move_mapping() */
	if (PageDirty(page))
		SetPageDirty(newpage);

	if (page_is_young(page))
		set_page_young(newpage);
	if (page_is_idle(page))
		set_page_idle(newpage);

	/*
	 * Copy NUMA information to the new page, to prevent over-eager
	 * future migrations of this same page.
	 */
	cpupid = page_cpupid_xchg_last(page, -1);
	page_cpupid_xchg_last(newpage, cpupid);

	ksm_migrate_page(newpage, page);
	/*
	 * Please do not reorder this without considering how mm/ksm.c's
	 * get_ksm_page() depends upon ksm_migrate_page() and PageSwapCache().
	 */
	if (PageSwapCache(page))
		ClearPageSwapCache(page);
	ClearPagePrivate(page);
	set_page_private(page, 0);

	/*
	 * If any waiters have accumulated on the new page then
	 * wake them up.
	 */
	if (PageWriteback(newpage))
		end_page_writeback(newpage);

	copy_page_owner(page, newpage);

	mem_cgroup_migrate(page, newpage);
}
