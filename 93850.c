 _TIFFmalloc(tmsize_t s)
 {
        if (s == 0)
                return ((void *) NULL);

 	return (malloc((size_t) s));
 }
