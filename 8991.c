output_colour(int colour, int fg_bg, int use_tc, int truecol, char *buf)
{
    int atrlen = 3, len;
    char *ptr = buf;
    if (buf) {
	strcpy(ptr, fg_bg == COL_SEQ_FG ? "fg=" : "bg=");
	ptr += 3;
    }
    if (truecol) {
	/* length of hex triplet always 7, don't need sprintf to count */
	atrlen += buf ? sprintf(ptr, "#%02x%02x%02x", colour >> 16,
		(colour >> 8) & 0xff, colour & 0xff) : 7;
    /* colour should only be > 7 if using termcap but let's be safe. Update:
     * currently other places in code don't always imply that colour > 7 =>
     * using-termcap - if zle_highlight will be non-default, then it will be
     * used instead of termcap even for colour > 7. Here this just emits the
     * color number, so it works fine for both zle_highlight and tercap cases
     */
    } else if (use_tc || colour > 7) {
	char digbuf[DIGBUFSIZE];
	sprintf(digbuf, "%d", colour);
	len = strlen(digbuf);
	atrlen += len;
	if (buf)
	    strcpy(ptr, digbuf);
    } else {
	len = strlen(ansi_colours[colour]);
	atrlen += len;
	if (buf)
	    strcpy(ptr, ansi_colours[colour]);
    }

    return atrlen;
}