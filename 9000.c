prompttrunc(int arg, int truncchar, int doprint, int endchar,
	    zattr *txtchangep)
{
    if (arg > 0) {
	char ch = *bv->fm, *ptr, *truncstr;
	int truncatleft = ch == '<';
	int w = bv->bp - bv->buf;

	/*
	 * If there is already a truncation active, return so that
	 * can be finished, backing up so that the new truncation
	 * can be started afterwards.
	 */
	if (bv->truncwidth) {
	    while (*--bv->fm != '%')
		;
	    bv->fm--;
	    return 0;
	}

	bv->truncwidth = arg;
	if (*bv->fm != ']')
	    bv->fm++;
	while (*bv->fm && *bv->fm != truncchar) {
	    if (*bv->fm == '\\' && bv->fm[1])
		++bv->fm;
	    addbufspc(1);
	    *bv->bp++ = *bv->fm++;
	}
	if (!*bv->fm)
	    return 0;
	if (bv->bp - bv->buf == w && truncchar == ']') {
	    addbufspc(1);
	    *bv->bp++ = '<';
	}
	ptr = bv->buf + w;	/* addbufspc() may have realloc()'d bv->buf */
	/*
	 * Now:
	 *   bv->buf is the start of the output prompt buffer
	 *   ptr is the start of the truncation string
	 *   bv->bp is the end of the truncation string
	 */
	truncstr = ztrduppfx(ptr, bv->bp - ptr);

	bv->bp = ptr;
	w = bv->bp - bv->buf;
	bv->fm++;
	bv->trunccount = bv->dontcount;
	putpromptchar(doprint, endchar, txtchangep);
	bv->trunccount = 0;
	ptr = bv->buf + w;	/* putpromptchar() may have realloc()'d */
	*bv->bp = '\0';
	/*
	 * Now:
	 *   ptr is the start of the truncation string and also
	 *     where we need to start putting any truncated output
	 *   bv->bp is the end of the string we have just added, which
	 *     may need truncating.
	 */

	/*
	 * w below is screen width if multibyte support is enabled
	 * (note that above it was a raw string pointer difference).
	 * It's the full width of the string we may need to truncate.
	 *
	 * bv->truncwidth has come from the user, so we interpret this
	 * as a screen width, too.
	 */
	countprompt(ptr, &w, 0, -1);
	if (w > bv->truncwidth) {
	    /*
	     * We need to truncate.  t points to the truncation string
	     * -- which is inserted literally, without nice
	     * representation.  twidth is its printing width, and maxwidth
	     * is the amount of the main string that we want to keep.
	     * Note that if the truncation string is longer than the
	     * truncation length (twidth > bv->truncwidth), the truncation
	     * string is used in full.
	     */
	    char *t = truncstr;
	    int fullen = bv->bp - ptr;
	    int twidth, maxwidth;
	    int ntrunc = strlen(t);

	    twidth = MB_METASTRWIDTH(t);
	    if (twidth < bv->truncwidth) {
		maxwidth = bv->truncwidth - twidth;
		/*
		 * It's not safe to assume there are no invisible substrings
		 * just because the width is less than the full string
		 * length since there may be multibyte characters.
		 */
		addbufspc(ntrunc+1);
		/* may have realloc'd */
		ptr = bv->bp - fullen;

		if (truncatleft) {
		    /*
		     * To truncate at the left, selectively copy
		     * maxwidth bytes from the main prompt, preceded
		     * by the truncation string in full.
		     *
		     * We're overwriting the string containing the
		     * text to be truncated, so copy it.  We've
		     * just ensured there's sufficient space at the
		     * end of the prompt string.
		     *
		     * Pointer into text to be truncated.
		     */
		    char *fulltextptr, *fulltext;
		    int remw;
#ifdef MULTIBYTE_SUPPORT
		    mbstate_t mbs;
		    memset(&mbs, 0, sizeof mbs);
#endif

		    fulltextptr = fulltext = ptr + ntrunc;
		    memmove(fulltext, ptr, fullen);
		    fulltext[fullen] = '\0';

		    /* Copy the truncstr into place. */
		    while (*t)
			*ptr++ = *t++;

		    /*
		     * Find the point in the text at which we should
		     * start copying, i.e. when the remaining width
		     * is less than or equal to the maximum width.
		     */
		    remw = w;
		    while (remw > maxwidth && *fulltextptr) {
			if (*fulltextptr == Inpar) {
			    /*
			     * Text marked as invisible: copy
			     * regardless, since we don't know what
			     * this does.  It only affects the width
			     * if there are Nularg's present.
			     * However, even in that case we
			     * can't break the sequence down, so
			     * we still loop over the entire group.
			     */
			    for (;;) {
				*ptr++ = *fulltextptr;
				if (*fulltextptr == '\0' ||
				    *fulltextptr++ == Outpar)
				    break;
				if (fulltextptr[-1] == Nularg)
				    remw--;
			    }
			} else {
#ifdef MULTIBYTE_SUPPORT
			    /*
			     * Normal text: build up a multibyte character.
			     */
			    char inchar;
			    wchar_t cc;
			    int wcw;

			    /*
			     * careful: string is still metafied (we
			     * need that because we don't know a
			     * priori when to stop and the resulting
			     * string must be metafied).
			     */
			    if (*fulltextptr == Meta)
				inchar = *++fulltextptr ^ 32;
			    else
				inchar = *fulltextptr;
			    fulltextptr++;
			    switch (mbrtowc(&cc, &inchar, 1, &mbs)) {
			    case MB_INCOMPLETE:
				/* Incomplete multibyte character. */
				break;
			    case MB_INVALID:
				/* Reset invalid state. */
				memset(&mbs, 0, sizeof mbs);
				/* FALL THROUGH */
			    case 0:
				/* Assume a single-byte character. */
				remw--;
				break;
			    default:
				wcw = WCWIDTH(cc);
				if (wcw >= 0)
				    remw -= wcw;
				else
				    remw--;
				break;
			    }
#else
			    /* Single byte character */
			    if (*fulltextptr == Meta)
				fulltextptr++;
			    fulltextptr++;
			    remw--;
#endif
			}
		    }

		    /*
		     * Now simply copy the rest of the text.  Still
		     * metafied, so this is easy.
		     */
		    while (*fulltextptr)
			*ptr++ = *fulltextptr++;
		    /* Mark the end of copying */
		    bv->bp = ptr;
		} else {
		    /*
		     * Truncating at the right is easier: just leave
		     * enough characters until we have reached the
		     * maximum width.
		     */
		    char *skiptext = ptr;
#ifdef MULTIBYTE_SUPPORT
		    mbstate_t mbs;
		    memset(&mbs, 0, sizeof mbs);
#endif

		    while (maxwidth > 0 && *skiptext) {
			if (*skiptext == Inpar) {
			    /* see comment on left truncation above */
			    for (;;) {
				if (*skiptext == '\0' ||
				    *skiptext++ == Outpar)
				    break;
				if (skiptext[-1] == Nularg)
				    maxwidth--;
			    }
			} else {
#ifdef MULTIBYTE_SUPPORT
			    char inchar;
			    wchar_t cc;
			    int wcw;

			    if (*skiptext == Meta)
				inchar = *++skiptext ^ 32;
			    else
				inchar = *skiptext;
			    skiptext++;
			    switch (mbrtowc(&cc, &inchar, 1, &mbs)) {
			    case MB_INCOMPLETE:
				/* Incomplete character. */
				break;
			    case MB_INVALID:
				/* Reset invalid state. */
				memset(&mbs, 0, sizeof mbs);
				/* FALL THROUGH */
			    case 0:
				/* Assume a single-byte character. */
				maxwidth--;
				break;
			    default:
				wcw = WCWIDTH(cc);
				if (wcw >= 0)
				    maxwidth -= wcw;
				else
				    maxwidth--;
				break;
			    }
#else
			    if (*skiptext == Meta)
				skiptext++;
			    skiptext++;
			    maxwidth--;
#endif
			}
		    }
		    /*
		     * We don't need the visible text from now on,
		     * but we'd better copy any invisible bits.
		     * History dictates that these go after the
		     * truncation string.  This is sensible since
		     * they may, for example, turn off an effect which
		     * should apply to all text at this point.
		     *
		     * Copy the truncstr.
		     */
		    ptr = skiptext;
		    while (*t)
			*ptr++ = *t++;
		    bv->bp = ptr;
		    if (*skiptext) {
			/* Move remaining text so we don't overwrite it */
			memmove(bv->bp, skiptext, strlen(skiptext)+1);
			skiptext = bv->bp;

			/*
			 * Copy anything we want, updating bv->bp
			 */
			while (*skiptext) {
			    if (*skiptext == Inpar) {
				for (;;) {
				    *bv->bp++ = *skiptext;
				    if (*skiptext == Outpar ||
					*skiptext == '\0')
					break;
				    skiptext++;
				}
			    }
			    else
				skiptext++;
			}
		    }
		}
	    } else {
		/* Just copy truncstr; no other text appears. */
		while (*t)
		    *ptr++ = *t++;
		bv->bp = ptr;
	    }
	    *bv->bp = '\0';
	}
	zsfree(truncstr);
	bv->truncwidth = 0;
	/*
	 * We may have returned early from the previous putpromptchar *
	 * because we found another truncation following this one.    *
	 * In that case we need to do the rest now.                   *
	 */
	if (!*bv->fm)
	    return 0;
	if (*bv->fm != endchar) {
	    bv->fm++;
	    /*
	     * With bv->truncwidth set to zero, we always reach endchar *
	     * (or the terminating NULL) this time round.         *
	     */
	    if (!putpromptchar(doprint, endchar, txtchangep))
		return 0;
	}
	/* Now we have to trick it into matching endchar again */
	bv->fm--;
    } else {
	if (*bv->fm != endchar)
	    bv->fm++;
	while(*bv->fm && *bv->fm != truncchar) {
	    if (*bv->fm == '\\' && bv->fm[1])
		bv->fm++;
	    bv->fm++;
	}
	if (bv->truncwidth || !*bv->fm)
	    return 0;
    }
    return 1;
}