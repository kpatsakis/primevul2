static int get_iovec_page_array(const struct iovec __user *iov,
				unsigned int nr_vecs, struct page **pages,
				struct partial_page *partial, bool aligned,
				unsigned int pipe_buffers)
{
	int buffers = 0, error = 0;

	while (nr_vecs) {
		unsigned long off, npages;
		struct iovec entry;
		void __user *base;
		size_t len;
		int i;

		error = -EFAULT;
		if (copy_from_user(&entry, iov, sizeof(entry)))
			break;

		base = entry.iov_base;
		len = entry.iov_len;

		/*
		 * Sanity check this iovec. 0 read succeeds.
		 */
		error = 0;
		if (unlikely(!len))
			break;
		error = -EFAULT;
		if (!access_ok(VERIFY_READ, base, len))
			break;

		/*
		 * Get this base offset and number of pages, then map
		 * in the user pages.
		 */
		off = (unsigned long) base & ~PAGE_MASK;

		/*
		 * If asked for alignment, the offset must be zero and the
		 * length a multiple of the PAGE_SIZE.
		 */
		error = -EINVAL;
		if (aligned && (off || len & ~PAGE_MASK))
			break;

		npages = (off + len + PAGE_SIZE - 1) >> PAGE_SHIFT;
		if (npages > pipe_buffers - buffers)
			npages = pipe_buffers - buffers;

		error = get_user_pages_fast((unsigned long)base, npages,
					0, &pages[buffers]);

		if (unlikely(error <= 0))
			break;

		/*
		 * Fill this contiguous range into the partial page map.
		 */
		for (i = 0; i < error; i++) {
			const int plen = min_t(size_t, len, PAGE_SIZE - off);

			partial[buffers].offset = off;
			partial[buffers].len = plen;

			off = 0;
			len -= plen;
			buffers++;
		}

		/*
		 * We didn't complete this iov, stop here since it probably
		 * means we have to move some of this into a pipe to
		 * be able to continue.
		 */
		if (len)
			break;

		/*
		 * Don't continue if we mapped fewer pages than we asked for,
		 * or if we mapped the max number of pages that we have
		 * room for.
		 */
		if (error < npages || buffers == pipe_buffers)
			break;

		nr_vecs--;
		iov++;
	}

	if (buffers)
		return buffers;

	return error;
}
