countprompt(char *str, int *wp, int *hp, int overf)
{
    int w = 0, h = 1, multi = 0, wcw = 0;
    int s = 1;
#ifdef MULTIBYTE_SUPPORT
    char inchar;
    mbstate_t mbs;
    wchar_t wc;

    memset(&mbs, 0, sizeof(mbs));
#endif

    for (; *str; str++) {
	/*
	 * Avoid double-incrementing the height when there's a newline in the
	 * prompt and the line it terminates takes up exactly the width of the
	 * terminal
	 */
	while (w > zterm_columns && overf >= 0 && !multi) {
	    h++;
	    if (wcw) {
		/*
		 * Wide characters don't get split off. They move to the
		 * next line if there is not enough space.
		 */
		w = wcw;
		break;
	    } else {
		/*
		 * Tabs overflow to the next line as if they were made of spaces.
		 */
		w -= zterm_columns;
	    }
	}
	wcw = 0;
	/*
	 * Input string should be metafied, so tokens in it should
	 * be real tokens, even if there are multibyte characters.
	 */
	if (*str == Inpar)
	    s = 0;
	else if (*str == Outpar)
	    s = 1;
	else if (*str == Nularg)
	    w++;
	else if (s) {
	    if (*str == Meta) {
#ifdef MULTIBYTE_SUPPORT
		inchar = *++str ^ 32;
#else
		str++;
#endif
	    } else {
#ifdef MULTIBYTE_SUPPORT
		/*
		 * Don't look for tab or newline in the middle
		 * of a multibyte character.  Otherwise, we are
		 * relying on the character set being an extension
		 * of ASCII so it's safe to test a single byte.
		 */
		if (!multi) {
#endif
		    if (*str == '\t') {
			w = (w | 7) + 1;
			continue;
		    } else if (*str == '\n') {
			w = 0;
			h++;
			continue;
		    }
#ifdef MULTIBYTE_SUPPORT
		}

		inchar = *str;
#endif
	    }

#ifdef MULTIBYTE_SUPPORT
	    switch (mbrtowc(&wc, &inchar, 1, &mbs)) {
	    case MB_INCOMPLETE:
		/* Character is incomplete -- keep looking. */
		multi = 1;
		break;
	    case MB_INVALID:
		memset(&mbs, 0, sizeof mbs);
		/* Invalid character: assume single width. */
		multi = 0;
		w++;
		break;
	    case 0:
		multi = 0;
		break;
	    default:
		/*
		 * If the character isn't printable, WCWIDTH() returns
		 * -1.  We assume width 1.
		 */
		wcw = WCWIDTH(wc);
		if (wcw >= 0)
		    w += wcw;
		else
		    w++;
		multi = 0;
		break;
	    }
#else
	    w++;
#endif
	}
    }
    /*
     * multi may still be set if we were in the middle of the character.
     * This isn't easy to handle generally; just assume there's no
     * output.
     */
    while (w > zterm_columns && overf >= 0) {
	h++;
	if (wcw) {
	    w = wcw;
	    break;
	} else {
	    w -= zterm_columns;
	}
    }
    if (w == zterm_columns && overf == 0) {
	w = 0;
	h++;
    }
    if(wp)
	*wp = w;
    if(hp)
	*hp = h;
}