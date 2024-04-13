unsigned long iov_iter_gap_alignment(const struct iov_iter *i)
{
	unsigned long res = 0;
	size_t size = i->count;

	if (unlikely(i->type & ITER_PIPE)) {
		WARN_ON(1);
		return ~0U;
	}

	iterate_all_kinds(i, size, v,
		(res |= (!res ? 0 : (unsigned long)v.iov_base) |
			(size != v.iov_len ? size : 0), 0),
		(res |= (!res ? 0 : (unsigned long)v.bv_offset) |
			(size != v.bv_len ? size : 0)),
		(res |= (!res ? 0 : (unsigned long)v.iov_base) |
			(size != v.iov_len ? size : 0))
		);
	return res;
}
