int jas_stream_read(jas_stream_t *stream, void *buf, int cnt)
{
	int n;
 	int c;
 	char *bufptr;
 
	if (cnt < 0) {
		jas_deprecated("negative count for jas_stream_read");
	}

 	bufptr = buf;
 
 	n = 0;
	while (n < cnt) {
		if ((c = jas_stream_getc(stream)) == EOF) {
			return n;
		}
		*bufptr++ = c;
		++n;
	}

	return n;
}