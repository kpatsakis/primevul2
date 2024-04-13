 static void pipe_advance(struct iov_iter *i, size_t size)
 {
 	struct pipe_inode_info *pipe = i->pipe;
 	if (unlikely(i->count < size))
 		size = i->count;
 	if (size) {
		struct pipe_buffer *buf;
		size_t off = i->iov_offset, left = size;
		int idx = i->idx;
 		if (off) /* make it relative to the beginning of buffer */
			left += off - pipe->bufs[idx].offset;
 		while (1) {
 			buf = &pipe->bufs[idx];
			if (left <= buf->len)
 				break;
			left -= buf->len;
 			idx = next_idx(idx, pipe);
 		}
 		i->idx = idx;
		i->iov_offset = buf->offset + left;
 	}
	i->count -= size;
	/* ... and discard everything past that point */
	pipe_truncate(i);
 }
