 int devmem_is_allowed(unsigned long pagenr)
 {
	if (page_is_ram(pagenr)) {
		/*
		 * For disallowed memory regions in the low 1MB range,
		 * request that the page be shown as all zeros.
		 */
		if (pagenr < 256)
			return 2;

		return 0;
	}

	/*
	 * This must follow RAM test, since System RAM is considered a
	 * restricted resource under CONFIG_STRICT_IOMEM.
	 */
	if (iomem_is_exclusive(pagenr << PAGE_SHIFT)) {
		/* Low 1MB bypasses iomem restrictions. */
		if (pagenr < 256)
			return 1;

 		return 0;
	}

	return 1;
 }
