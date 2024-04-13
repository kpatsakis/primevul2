output_highlight(zattr atr, char *buf)
{
    const struct highlight *hp;
    int atrlen = 0, len;
    char *ptr = buf;

    if (atr & TXTFGCOLOUR) {
	len = output_colour(txtchangeget(atr, TXT_ATTR_FG_COL),
			    COL_SEQ_FG,
			    (atr & TXT_ATTR_FG_TERMCAP),
			    (atr & TXT_ATTR_FG_24BIT),
			    ptr);
	atrlen += len;
	if (buf)
	    ptr += len;
    }
    if (atr & TXTBGCOLOUR) {
	if (atrlen) {
	    atrlen++;
	    if (buf) {
		strcpy(ptr, ",");
		ptr++;
	    }
	}
	len = output_colour(txtchangeget(atr, TXT_ATTR_BG_COL),
			    COL_SEQ_BG,
			    (atr & TXT_ATTR_BG_TERMCAP),
			    (atr & TXT_ATTR_BG_24BIT),
			    ptr);
	atrlen += len;
	if (buf)
	    ptr += len;
    }
    for (hp = highlights; hp->name; hp++) {
	if (hp->mask_on & atr) {
	    if (atrlen) {
		atrlen++;
		if (buf) {
		    strcpy(ptr, ",");
		    ptr++;
		}
	    }
	    len = strlen(hp->name);
	    atrlen += len;
	    if (buf) {
		strcpy(ptr, hp->name);
		ptr += len;
	    }
	}
    }

    if (atrlen == 0) {
	if (buf)
	    strcpy(ptr, "none");
	return 4;
    }
    return atrlen;
}