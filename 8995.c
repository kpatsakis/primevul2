parsecolorchar(zattr arg, int is_fg)
{
    if (bv->fm[1] == '{') {
	char *ep;
	bv->fm += 2; /* skip over F{ */
	if ((ep = strchr(bv->fm, '}'))) {
	    char oc = *ep, *col, *coll;
	    int ops = opts[PROMPTSUBST], opb = opts[PROMPTBANG];
	    int opp = opts[PROMPTPERCENT];

	    opts[PROMPTPERCENT] = 1;
	    opts[PROMPTSUBST] = opts[PROMPTBANG] = 0;

	    *ep = '\0';
	    /* expand the contents of the argument so you can use
	     * %v for example */
	    coll = col = promptexpand(bv->fm, 0, NULL, NULL, NULL);
	    *ep = oc;
	    arg = match_colour((const char **)&coll, is_fg, 0);
	    free(col);
	    bv->fm = ep;

	    opts[PROMPTSUBST] = ops;
	    opts[PROMPTBANG] = opb;
	    opts[PROMPTPERCENT] = opp;
	} else {
	    arg = match_colour((const char **)&bv->fm, is_fg, 0);
	    if (*bv->fm != '}')
		bv->fm--;
	}
    } else
	arg = match_colour(NULL, 1, arg);
    return arg;
}