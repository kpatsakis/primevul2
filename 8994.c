match_colour(const char **teststrp, int is_fg, int colour)
{
    int shft, named = 0, tc;
    zattr on;

    if (is_fg) {
	shft = TXT_ATTR_FG_COL_SHIFT;
	on = TXTFGCOLOUR;
	tc = TCFGCOLOUR;
    } else {
	shft = TXT_ATTR_BG_COL_SHIFT;
	on = TXTBGCOLOUR;
	tc = TCBGCOLOUR;
    }
    if (teststrp) {
	if (**teststrp == '#' && isxdigit((*teststrp)[1])) {
	    struct color_rgb color;
	    char *end;
	    zlong col = zstrtol(*teststrp+1, &end, 16);
            if (end - *teststrp == 4) {
		color.red = col >> 8 | ((col >> 8) << 4);
		color.green = (col & 0xf0) >> 4;
		color.green |= color.green << 4;
		color.blue = col & 0xf;
		color.blue |= color.blue << 4;
	    } else if (end - *teststrp == 7) {
		color.red = col >> 16;
		color.green = (col & 0xff00) >> 8;
		color.blue = col & 0xff;
	    } else
		return TXT_ERROR;
	    *teststrp = end;
	    colour = runhookdef(GETCOLORATTR, &color) - 1;
	    if (colour == -1) { /* no hook function added, try true color (24-bit) */
		colour = (((color.red << 8) + color.green) << 8) + color.blue;
		return on | (is_fg ? TXT_ATTR_FG_24BIT : TXT_ATTR_BG_24BIT) |
			(zattr)colour << shft;
	    } else if (colour <= -2) {
		return TXT_ERROR;
	    }
	} else if ((named = ialpha(**teststrp))) {
	    colour = match_named_colour(teststrp);
	    if (colour == 8) {
		/* default */
		return is_fg ? TXTNOFGCOLOUR : TXTNOBGCOLOUR;
	    }
	    if (colour < 0)
		return TXT_ERROR;
	}
	else {
	    colour = (int)zstrtol(*teststrp, (char **)teststrp, 10);
	    if (colour < 0 || colour >= 256)
		return TXT_ERROR;
	}
    }
    /*
     * Try termcap for numbered characters if possible.
     * Don't for named characters, since our best bet
     * of getting the names right is with ANSI sequences.
     */
    if (!named && tccan(tc)) {
	if (tccolours >= 0 && colour >= tccolours) {
	    /*
	     * Out of range of termcap colours.
	     * Can we assume ANSI colours work?
	     */
	    if (colour > 7)
		return TXT_ERROR; /* No. */
	} else {
	    /*
	     * We can handle termcap colours and the number
	     * is in range, so use termcap.
	     */
	    on |= is_fg ? TXT_ATTR_FG_TERMCAP :
		TXT_ATTR_BG_TERMCAP;
	}
    }
    return on | (zattr)colour << shft;
}