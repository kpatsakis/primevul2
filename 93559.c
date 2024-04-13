 int jas_stream_gobble(jas_stream_t *stream, int n)
 {
 	int m;
	if (n < 0) {
		jas_deprecated("negative count for jas_stream_gobble");
	}
 	m = n;
 	for (m = n; m > 0; --m) {
 		if (jas_stream_getc(stream) == EOF) {
			return n - m;
		}
	}
	return n;
}
