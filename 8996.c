putpromptchar(int doprint, int endchar, zattr *txtchangep)
{
    char *ss, *hostnam;
    int t0, arg, test, sep, j, numjobs, len;
    zattr atr;
    struct tm *tm;
    struct timespec ts;
    time_t timet;
    Nameddir nd;

    for (; *bv->fm && *bv->fm != endchar; bv->fm++) {
	arg = 0;
	if (*bv->fm == '%' && isset(PROMPTPERCENT)) {
	    int minus = 0;
	    bv->fm++;
	    if (*bv->fm == '-') {
		minus = 1;
		bv->fm++;
	    }
	    if (idigit(*bv->fm)) {
		arg = zstrtol(bv->fm, &bv->fm, 10);
		if (minus)
		    arg *= -1;
	    } else if (minus)
		arg = -1;
	    if (*bv->fm == '(') {
		int tc, otruncwidth;

		if (idigit(*++bv->fm)) {
		    arg = zstrtol(bv->fm, &bv->fm, 10);
		} else if (arg < 0) {
		    /* negative numbers don't make sense here */
		    arg *= -1;
		}
		test = 0;
		ss = pwd;
		switch (tc = *bv->fm) {
		case 'c':
		case '.':
		case '~':
		    if ((nd = finddir(ss))) {
			arg--;
			ss += strlen(nd->dir);
		    } /*FALLTHROUGH*/
		case '/':
		case 'C':
		    /* `/' gives 0, `/any' gives 1, etc. */
		    if (*ss && *ss++ == '/' && *ss)
			arg--;
		    for (; *ss; ss++)
			if (*ss == '/')
			    arg--;
		    if (arg <= 0)
			test = 1;
		    break;
		case 't':
		case 'T':
		case 'd':
		case 'D':
		case 'w':
		    timet = time(NULL);
		    tm = localtime(&timet);
		    switch (tc) {
		    case 't':
			test = (arg == tm->tm_min);
			break;
		    case 'T':
			test = (arg == tm->tm_hour);
			break;
		    case 'd':
			test = (arg == tm->tm_mday);
			break;
		    case 'D':
			test = (arg == tm->tm_mon);
			break;
		    case 'w':
			test = (arg == tm->tm_wday);
			break;
		    }
		    break;
		case '?':
		    if (lastval == arg)
			test = 1;
		    break;
		case '#':
		    if (geteuid() == (uid_t)arg)
			test = 1;
		    break;
		case 'g':
		    if (getegid() == (gid_t)arg)
			test = 1;
		    break;
		case 'j':
		    for (numjobs = 0, j = 1; j <= maxjob; j++)
			if (jobtab[j].stat && jobtab[j].procs &&
		    	    !(jobtab[j].stat & STAT_NOPRINT)) numjobs++;
		    if (numjobs >= arg)
		    	test = 1;
		    break;
		case 'l':
		    *bv->bp = '\0';
		    countprompt(bv->bufline, &t0, 0, 0);
		    if (minus)
			t0 = zterm_columns - t0;
		    if (t0 >= arg)
			test = 1;
		    break;
		case 'e':
		    {
			Funcstack fsptr = funcstack;
			test = arg;
			while (fsptr && test > 0) {
			    test--;
			    fsptr = fsptr->prev;
			}
			test = !test;
		    }
		    break;
		case 'L':
		    if (shlvl >= arg)
			test = 1;
		    break;
		case 'S':
		    if (time(NULL) - shtimer.tv_sec >= arg)
			test = 1;
		    break;
		case 'v':
		    if (arrlen_ge(psvar, arg))
			test = 1;
		    break;
		case 'V':
		    if (psvar && *psvar && arrlen_ge(psvar, arg)) {
			if (*psvar[(arg ? arg : 1) - 1])
			    test = 1;
		    }
		    break;
		case '_':
		    test = (cmdsp >= arg);
		    break;
		case '!':
		    test = privasserted();
		    break;
		default:
		    test = -1;
		    break;
		}
		if (!*bv->fm || !(sep = *++bv->fm))
		    return 0;
		bv->fm++;
		/* Don't do the current truncation until we get back */
		otruncwidth = bv->truncwidth;
		bv->truncwidth = 0;
		if (!putpromptchar(test == 1 && doprint, sep,
				   txtchangep) || !*++bv->fm ||
		    !putpromptchar(test == 0 && doprint, ')',
				   txtchangep)) {
		    bv->truncwidth = otruncwidth;
		    return 0;
		}
		bv->truncwidth = otruncwidth;
		continue;
	    }
	    if (!doprint)
		switch(*bv->fm) {
		  case '[':
		    while(idigit(*++bv->fm));
		    while(*++bv->fm != ']');
		    continue;
		  case '<':
		    while(*++bv->fm != '<');
		    continue;
		  case '>':
		    while(*++bv->fm != '>');
		    continue;
		  case 'D':
		    if(bv->fm[1]=='{')
			while(*++bv->fm != '}');
		    continue;
		  default:
		    continue;
		}
	    switch (*bv->fm) {
	    case '~':
		promptpath(pwd, arg, 1);
		break;
	    case 'd':
	    case '/':
		promptpath(pwd, arg, 0);
		break;
	    case 'c':
	    case '.':
		promptpath(pwd, arg ? arg : 1, 1);
		break;
	    case 'C':
		promptpath(pwd, arg ? arg : 1, 0);
		break;
	    case 'N':
		promptpath(scriptname ? scriptname : argzero, arg, 0);
		break;
	    case 'h':
	    case '!':
		addbufspc(DIGBUFSIZE);
		convbase(bv->bp, curhist, 10);
		bv->bp += strlen(bv->bp);
		break;
	    case 'j':
		for (numjobs = 0, j = 1; j <= maxjob; j++)
		    if (jobtab[j].stat && jobtab[j].procs &&
		    	!(jobtab[j].stat & STAT_NOPRINT)) numjobs++;
		addbufspc(DIGBUFSIZE);
		sprintf(bv->bp, "%d", numjobs);
		bv->bp += strlen(bv->bp);
		break;
	    case 'M':
		queue_signals();
		if ((hostnam = getsparam("HOST")))
		    stradd(hostnam);
		unqueue_signals();
		break;
	    case 'm':
		if (!arg)
		    arg++;
		queue_signals();
		if (!(hostnam = getsparam("HOST"))) {
		    unqueue_signals();
		    break;
		}
		if (arg < 0) {
		    for (ss = hostnam + strlen(hostnam); ss > hostnam; ss--)
			if (ss[-1] == '.' && !++arg)
			    break;
		    stradd(ss);
		} else {
		    for (ss = hostnam; *ss; ss++)
			if (*ss == '.' && !--arg)
			    break;
		    stradd(*ss ? dupstrpfx(hostnam, ss - hostnam) : hostnam);
		}
		unqueue_signals();
		break;
	    case 'S':
		txtchangeset(txtchangep, TXTSTANDOUT, TXTNOSTANDOUT);
		txtset(TXTSTANDOUT);
		tsetcap(TCSTANDOUTBEG, TSC_PROMPT);
		break;
	    case 's':
		txtchangeset(txtchangep, TXTNOSTANDOUT, TXTSTANDOUT);
		txtunset(TXTSTANDOUT);
		tsetcap(TCSTANDOUTEND, TSC_PROMPT|TSC_DIRTY);
		break;
	    case 'B':
		txtchangeset(txtchangep, TXTBOLDFACE, TXTNOBOLDFACE);
		txtset(TXTBOLDFACE);
		tsetcap(TCBOLDFACEBEG, TSC_PROMPT|TSC_DIRTY);
		break;
	    case 'b':
		txtchangeset(txtchangep, TXTNOBOLDFACE, TXTBOLDFACE);
		txtunset(TXTBOLDFACE);
		tsetcap(TCALLATTRSOFF, TSC_PROMPT|TSC_DIRTY);
		break;
	    case 'U':
		txtchangeset(txtchangep, TXTUNDERLINE, TXTNOUNDERLINE);
		txtset(TXTUNDERLINE);
		tsetcap(TCUNDERLINEBEG, TSC_PROMPT);
		break;
	    case 'u':
		txtchangeset(txtchangep, TXTNOUNDERLINE, TXTUNDERLINE);
		txtunset(TXTUNDERLINE);
		tsetcap(TCUNDERLINEEND, TSC_PROMPT|TSC_DIRTY);
		break;
	    case 'F':
		atr = parsecolorchar(arg, 1);
		if (!(atr & (TXT_ERROR | TXTNOFGCOLOUR))) {
		    txtchangeset(txtchangep, atr & TXT_ATTR_FG_ON_MASK,
				 TXTNOFGCOLOUR | TXT_ATTR_FG_COL_MASK);
		    txtunset(TXT_ATTR_FG_COL_MASK);
		    txtset(atr & TXT_ATTR_FG_ON_MASK);
		    set_colour_attribute(atr, COL_SEQ_FG, TSC_PROMPT);
		    break;
		}
		/* else FALLTHROUGH */
	    case 'f':
		txtchangeset(txtchangep, TXTNOFGCOLOUR, TXT_ATTR_FG_ON_MASK);
		txtunset(TXT_ATTR_FG_ON_MASK);
		set_colour_attribute(TXTNOFGCOLOUR, COL_SEQ_FG, TSC_PROMPT);
		break;
	    case 'K':
		atr = parsecolorchar(arg, 0);
		if (!(atr & (TXT_ERROR | TXTNOBGCOLOUR))) {
		    txtchangeset(txtchangep, atr & TXT_ATTR_BG_ON_MASK,
				 TXTNOBGCOLOUR | TXT_ATTR_BG_COL_MASK);
		    txtunset(TXT_ATTR_BG_COL_MASK);
		    txtset(atr & TXT_ATTR_BG_ON_MASK);
		    set_colour_attribute(atr, COL_SEQ_BG, TSC_PROMPT);
		    break;
		}
		/* else FALLTHROUGH */
	    case 'k':
		txtchangeset(txtchangep, TXTNOBGCOLOUR, TXT_ATTR_BG_ON_MASK);
		txtunset(TXT_ATTR_BG_ON_MASK);
		set_colour_attribute(TXTNOBGCOLOUR, COL_SEQ_BG, TSC_PROMPT);
		break;
	    case '[':
		if (idigit(*++bv->fm))
		    arg = zstrtol(bv->fm, &bv->fm, 10);
		if (!prompttrunc(arg, ']', doprint, endchar, txtchangep))
		    return *bv->fm;
		break;
	    case '<':
	    case '>':
		/* Test (minus) here so -0 means "at the right margin" */
		if (minus) {
		    *bv->bp = '\0';
		    countprompt(bv->bufline, &t0, 0, 0);
		    arg = zterm_columns - t0 + arg;
		    if (arg <= 0)
			arg = 1;
		}
		if (!prompttrunc(arg, *bv->fm, doprint, endchar, txtchangep))
		    return *bv->fm;
		break;
	    case '{': /*}*/
		if (!bv->dontcount++) {
		    addbufspc(1);
		    *bv->bp++ = Inpar;
		}
		if (arg <= 0)
		    break;
		/* else */
		/* FALLTHROUGH */
	    case 'G':
		if (arg > 0) {
		    addbufspc(arg);
		    while (arg--)
			*bv->bp++ = Nularg;
		} else {
		    addbufspc(1);
		    *bv->bp++ = Nularg;
		}
		break;
	    case /*{*/ '}':
		if (bv->trunccount && bv->trunccount >= bv->dontcount)
		    return *bv->fm;
		if (bv->dontcount && !--bv->dontcount) {
		    addbufspc(1);
		    *bv->bp++ = Outpar;
		}
		break;
	    case 't':
	    case '@':
	    case 'T':
	    case '*':
	    case 'w':
	    case 'W':
	    case 'D':
		{
		    char *tmfmt, *dd, *tmbuf = NULL;

		    switch (*bv->fm) {
		    case 'T':
			tmfmt = "%K:%M";
			break;
		    case '*':
			tmfmt = "%K:%M:%S";
			break;
		    case 'w':
			tmfmt = "%a %f";
			break;
		    case 'W':
			tmfmt = "%m/%d/%y";
			break;
		    case 'D':
			if (bv->fm[1] == '{' /*}*/) {
			    for (ss = bv->fm + 2; *ss && *ss != /*{*/ '}'; ss++)
				if(*ss == '\\' && ss[1])
				    ss++;
			    dd = tmfmt = tmbuf = zalloc(ss - bv->fm);
			    for (ss = bv->fm + 2; *ss && *ss != /*{*/ '}';
				 ss++) {
				if(*ss == '\\' && ss[1])
				    ss++;
				*dd++ = *ss;
			    }
			    *dd = 0;
			    bv->fm = ss - !*ss;
			    if (!*tmfmt) {
				free(tmbuf);
				continue;
			    }
			} else
			    tmfmt = "%y-%m-%d";
			break;
		    default:
			tmfmt = "%l:%M%p";
			break;
		    }
		    zgettime(&ts);
		    tm = localtime(&ts.tv_sec);
		    /*
		     * Hack because strftime won't say how
		     * much space it actually needs.  Try to add it
		     * a few times until it works.  Some formats don't
		     * actually have a length, so we could go on for
		     * ever.
		     */
		    for(j = 0, t0 = strlen(tmfmt)*8; j < 3; j++, t0*=2) {
			addbufspc(t0);
			if ((len = ztrftime(bv->bp, t0, tmfmt, tm, ts.tv_nsec))
			    >= 0)
			    break;
		    }
		    /* There is enough room for this because addbufspc(t0)
		     * allocates room for t0 * 2 bytes. */
		    if (len >= 0)
			metafy(bv->bp, len, META_NOALLOC);
		    bv->bp += strlen(bv->bp);
		    zsfree(tmbuf);
		    break;
		}
	    case 'n':
		stradd(get_username());
		break;
	    case 'l':
		if (*ttystrname) {
                   ss = (strncmp(ttystrname, "/dev/tty", 8) ?
                           ttystrname + 5 : ttystrname + 8);
		    stradd(ss);
		} else
		    stradd("()");
		break;
	    case 'y':
		if (*ttystrname) {
		    ss = (strncmp(ttystrname, "/dev/", 5) ?
			    ttystrname : ttystrname + 5);
		    stradd(ss);
		} else
		    stradd("()");
		break;
	    case 'L':
		addbufspc(DIGBUFSIZE);
#if defined(ZLONG_IS_LONG_LONG) && defined(PRINTF_HAS_LLD)
		sprintf(bv->bp, "%lld", shlvl);
#else
		sprintf(bv->bp, "%ld", (long)shlvl);
#endif
		bv->bp += strlen(bv->bp);
		break;
	    case '?':
		addbufspc(DIGBUFSIZE);
#if defined(ZLONG_IS_LONG_LONG) && defined(PRINTF_HAS_LLD)
		sprintf(bv->bp, "%lld", lastval);
#else
		sprintf(bv->bp, "%ld", (long)lastval);
#endif
		bv->bp += strlen(bv->bp);
		break;
	    case '%':
	    case ')':
		addbufspc(1);
		*bv->bp++ = *bv->fm;
		break;
	    case '#':
		addbufspc(1);
		*bv->bp++ = privasserted() ? '#' : '%';
		break;
	    case 'v':
		if (!arg)
		    arg = 1;
		else if (arg < 0)
		    arg += arrlen(psvar) + 1;
		if (arg > 0 && arrlen_ge(psvar, arg))
		    stradd(psvar[arg - 1]);
		break;
	    case 'E':
                tsetcap(TCCLEAREOL, TSC_PROMPT);
		break;
	    case '^':
		if (cmdsp) {
		    if (arg >= 0) {
			if (arg > cmdsp || arg == 0)
			    arg = cmdsp;
			for (t0 = cmdsp - 1; arg--; t0--) {
			    stradd(cmdnames[cmdstack[t0]]);
			    if (arg) {
				addbufspc(1);
				*bv->bp++=' ';
			    }
			}
		    } else {
			arg = -arg;
			if (arg > cmdsp)
			    arg = cmdsp;
			for (t0 = arg - 1; arg--; t0--) {
			    stradd(cmdnames[cmdstack[t0]]);
			    if (arg) {
				addbufspc(1);
				*bv->bp++=' ';
			    }
			}
		    }
		}
		break;
	    case '_':
		if (cmdsp) {
		    if (arg >= 0) {
			if (arg > cmdsp || arg == 0)
			    arg = cmdsp;
			for (t0 = cmdsp - arg; arg--; t0++) {
			    stradd(cmdnames[cmdstack[t0]]);
			    if (arg) {
				addbufspc(1);
				*bv->bp++=' ';
			    }
			}
		    } else {
			arg = -arg;
			if (arg > cmdsp)
			    arg = cmdsp;
			for (t0 = 0; arg--; t0++) {
			    stradd(cmdnames[cmdstack[t0]]);
			    if (arg) {
				addbufspc(1);
				*bv->bp++=' ';
			    }
			}
		    }
		}
		break;
	    case 'r':
		if(bv->rstring)
		    stradd(bv->rstring);
		break;
	    case 'R':
		if(bv->Rstring)
		    stradd(bv->Rstring);
		break;
	    case 'e':
	    {
		int depth = 0;
		Funcstack fsptr = funcstack;
		while (fsptr) {
		    depth++;
		    fsptr = fsptr->prev;
		}
		addbufspc(DIGBUFSIZE);
		sprintf(bv->bp, "%d", depth);
		bv->bp += strlen(bv->bp);
		break;
	    }
	    case 'I':
		if (funcstack && funcstack->tp != FS_SOURCE &&
		    !IN_EVAL_TRAP()) {
		    /*
		     * We're in a function or an eval with
		     * EVALLINENO.  Calculate the line number in
		     * the file.
		     */
		    zlong flineno = lineno + funcstack->flineno;
		    /* take account of eval line nos. starting at 1 */
		    if (funcstack->tp == FS_EVAL)
			lineno--;
		    addbufspc(DIGBUFSIZE);
#if defined(ZLONG_IS_LONG_LONG) && defined(PRINTF_HAS_LLD)
		    sprintf(bv->bp, "%lld", flineno);
#else
		    sprintf(bv->bp, "%ld", (long)flineno);
#endif
		    bv->bp += strlen(bv->bp);
		    break;
		}
		/* else we're in a file and lineno is already correct */
		/* FALLTHROUGH */
	    case 'i':
		addbufspc(DIGBUFSIZE);
#if defined(ZLONG_IS_LONG_LONG) && defined(PRINTF_HAS_LLD)
		sprintf(bv->bp, "%lld", lineno);
#else
		sprintf(bv->bp, "%ld", (long)lineno);
#endif
		bv->bp += strlen(bv->bp);
		break;
	    case 'x':
		if (funcstack && funcstack->tp != FS_SOURCE &&
		    !IN_EVAL_TRAP())
		    promptpath(funcstack->filename ? funcstack->filename : "",
			       arg, 0);
		else
		    promptpath(scriptfilename ? scriptfilename : argzero,
			       arg, 0);
		break;
	    case '\0':
		return 0;
	    case Meta:
		bv->fm++;
		break;
	    }
	} else if(*bv->fm == '!' && isset(PROMPTBANG)) {
	    if(doprint) {
		if(bv->fm[1] == '!') {
		    bv->fm++;
		    addbufspc(1);
		    pputc('!');
		} else {
		    addbufspc(DIGBUFSIZE);
		    convbase(bv->bp, curhist, 10);
		    bv->bp += strlen(bv->bp);
		}
	    }
	} else {
	    char c = *bv->fm == Meta ? *++bv->fm ^ 32 : *bv->fm;

	    if (doprint) {
		addbufspc(1);
		pputc(c);
	    }
	}
    }

    return *bv->fm;
}