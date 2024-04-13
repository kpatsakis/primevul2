stradd(char *d)
{
#ifdef MULTIBYTE_SUPPORT
    char *ums, *ups;
    int upslen, eol = 0;
    mbstate_t mbs;

    memset(&mbs, 0, sizeof mbs);
    ums = ztrdup(d);
    ups = unmetafy(ums, &upslen);

    /*
     * We now have a raw string of possibly multibyte characters.
     * Read each character one by one.
     */
    while (upslen > 0) {
	wchar_t cc;
	char *pc;
	size_t cnt = eol ? MB_INVALID : mbrtowc(&cc, ups, upslen, &mbs);

	switch (cnt) {
	case MB_INCOMPLETE:
	    eol = 1;
	    /* FALL THROUGH */
	case MB_INVALID:
	    /* Bad character.  Take the next byte on its own. */
	    pc = nicechar(*ups);
	    cnt = 1;
	    memset(&mbs, 0, sizeof mbs);
	    break;
	case 0:
	    cnt = 1;
	    /* FALL THROUGH */
	default:
	    /* Take full wide character in one go */
	    mb_charinit();
	    pc = wcs_nicechar(cc, NULL, NULL);
	    break;
	}
	/* Keep output as metafied string. */
	addbufspc(strlen(pc));

	upslen -= cnt;
	ups += cnt;

	/* Put printed representation into the buffer */
	while (*pc)
	    *bv->bp++ = *pc++;
    }

    free(ums);
#else
    char *ps, *pc;
    addbufspc(niceztrlen(d));
    /* This loop puts the nice representation of the string into the
     * prompt buffer. */
    for (ps = d; *ps; ps++) {
	for (pc = nicechar(*ps == Meta ? *++ps^32 : *ps); *pc; pc++)
	    *bv->bp++ = *pc;
    }
#endif
}