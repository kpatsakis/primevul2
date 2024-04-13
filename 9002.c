set_colour_attribute(zattr atr, int fg_bg, int flags)
{
    char *ptr;
    int do_free, is_prompt = (flags & TSC_PROMPT) ? 1 : 0;
    int colour, tc, def, use_termcap, use_truecolor;
    int is_default_zle_highlight = 1;

    if (fg_bg == COL_SEQ_FG) {
	colour = txtchangeget(atr, TXT_ATTR_FG_COL);
	tc = TCFGCOLOUR;
	def = txtchangeisset(atr, TXTNOFGCOLOUR);
	use_truecolor = txtchangeisset(atr, TXT_ATTR_FG_24BIT);
	use_termcap = txtchangeisset(atr, TXT_ATTR_FG_TERMCAP);
    } else {
	colour = txtchangeget(atr, TXT_ATTR_BG_COL);
	tc = TCBGCOLOUR;
	def = txtchangeisset(atr, TXTNOBGCOLOUR);
	use_truecolor = txtchangeisset(atr, TXT_ATTR_BG_24BIT);
	use_termcap = txtchangeisset(atr, TXT_ATTR_BG_TERMCAP);
    }

    /* Test if current zle_highlight settings are customized, or
     * the typical "standard" codes */
    if (0 != strcmp(fg_bg_sequences[fg_bg].start, fg_bg == COL_SEQ_FG ? TC_COL_FG_START : TC_COL_BG_START) ||
	/* the same in-fix for both FG and BG */
	0 != strcmp(fg_bg_sequences[fg_bg].def, TC_COL_FG_DEFAULT) ||
	/* the same suffix for both FG and BG */
	0 != strcmp(fg_bg_sequences[fg_bg].end, TC_COL_FG_END))
    {
	is_default_zle_highlight = 0;
    }

    /*
     * If we're not restoring the default, and either have a
     * colour value that is too large for ANSI, or have been told
     * to use the termcap sequence, try to use the termcap sequence.
     * True color is not covered by termcap.
     *
     * We have already sanitised the values we allow from the
     * highlighting variables, so much of this shouldn't be
     * necessary at this point, but we might as well be safe.
     */
    if (!def && !use_truecolor &&
	(is_default_zle_highlight && (colour > 7 || use_termcap)))
    {
	/*
	 * We can if it's available, and either we couldn't get
	 * the maximum number of colours, or the colour is in range.
	 */
	if (tccan(tc) && (tccolours < 0 || colour < tccolours))
	{
	    if (is_prompt)
	    {
		if (!bv->dontcount) {
		    addbufspc(1);
		    *bv->bp++ = Inpar;
		}
		tputs(tgoto(tcstr[tc], colour, colour), 1, putstr);
		if (!bv->dontcount) {
		    addbufspc(1);
		    *bv->bp++ = Outpar;
		}
	    } else {
		tputs(tgoto(tcstr[tc], colour, colour), 1, putshout);
	    }
	    /* That worked. */
	    return;
	}
	/*
	 * Nope, that didn't work.
	 * If 0 to 7, assume standard ANSI works, if 8 to 255, assume
         * typical 256-color escapes works, otherwise it won't.
	 */
	if (colour > 255)
	    return;
    }

    if ((do_free = (colseq_buf == NULL))) {
	/* This can happen when moving the cursor in trashzle() */
	allocate_colour_buffer();
    }

    /* Build the reset-code: .start + .def + . end
     * or the typical true-color code: .start + 8;2;%d;%d;%d + .end
     * or the typical 256-color code: .start + 8;5;%d + .end
     */
    if (use_truecolor)
	strcpy(colseq_buf, fg_bg == COL_SEQ_FG ? TC_COL_FG_START : TC_COL_BG_START);
    else
	strcpy(colseq_buf, fg_bg_sequences[fg_bg].start);

    ptr = colseq_buf + strlen(colseq_buf);
    if (def) {
	if (use_truecolor)
	    strcpy(ptr, fg_bg == COL_SEQ_FG ? TC_COL_FG_DEFAULT : TC_COL_BG_DEFAULT);
	else
	    strcpy(ptr, fg_bg_sequences[fg_bg].def);
	while (*ptr)
	    ptr++;
    } else if (use_truecolor) {
	ptr += sprintf(ptr, "8;2;%d;%d;%d", colour >> 16,
		(colour >> 8) & 0xff, colour & 0xff);
    } else if (colour > 7 && colour <= 255) {
	ptr += sprintf(ptr, "%d", colour);
    } else
	*ptr++ = colour + '0';
    if (use_truecolor)
	strcpy(ptr, fg_bg == COL_SEQ_FG ? TC_COL_FG_END : TC_COL_BG_END);
    else
	strcpy(ptr, fg_bg_sequences[fg_bg].end);

    if (is_prompt) {
	if (!bv->dontcount) {
	    addbufspc(1);
	    *bv->bp++ = Inpar;
	}
	tputs(colseq_buf, 1, putstr);
	if (!bv->dontcount) {
	    addbufspc(1);
	    *bv->bp++ = Outpar;
	}
    } else
	tputs(colseq_buf, 1, putshout);

    if (do_free)
	free_colour_buffer();
}