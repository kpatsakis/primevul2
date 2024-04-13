tsetcap(int cap, int flags)
{
    if (tccan(cap) && !isset(SINGLELINEZLE) &&
        !(termflags & (TERM_NOUP|TERM_BAD|TERM_UNKNOWN))) {
	switch (flags & TSC_OUTPUT_MASK) {
	case TSC_RAW:
	    tputs(tcstr[cap], 1, putraw);
	    break;
	case 0:
	default:
	    tputs(tcstr[cap], 1, putshout);
	    break;
	case TSC_PROMPT:
	    if (!bv->dontcount) {
		addbufspc(1);
		*bv->bp++ = Inpar;
	    }
	    tputs(tcstr[cap], 1, putstr);
	    if (!bv->dontcount) {
		int glitch = 0;

		if (cap == TCSTANDOUTBEG || cap == TCSTANDOUTEND)
		    glitch = tgetnum("sg");
		else if (cap == TCUNDERLINEBEG || cap == TCUNDERLINEEND)
		    glitch = tgetnum("ug");
		if(glitch < 0)
		    glitch = 0;
		addbufspc(glitch + 1);
		while(glitch--)
		    *bv->bp++ = Nularg;
		*bv->bp++ = Outpar;
	    }
	    break;
	}

	if (flags & TSC_DIRTY) {
	    flags &= ~TSC_DIRTY;
	    if (txtisset(TXTBOLDFACE) && cap != TCBOLDFACEBEG)
		tsetcap(TCBOLDFACEBEG, flags);
	    if (txtisset(TXTSTANDOUT))
		tsetcap(TCSTANDOUTBEG, flags);
	    if (txtisset(TXTUNDERLINE))
		tsetcap(TCUNDERLINEBEG, flags);
	    if (txtisset(TXTFGCOLOUR))
		set_colour_attribute(txtattrmask, COL_SEQ_FG, TSC_PROMPT);
	    if (txtisset(TXTBGCOLOUR))
		set_colour_attribute(txtattrmask, COL_SEQ_BG, TSC_PROMPT);
	}
    }
}