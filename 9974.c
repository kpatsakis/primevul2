tabstop_fromto(
	colnr_T start_col,
	colnr_T end_col,
	int	ts_arg,
	int	*vts,
	int	*ntabs,
	int	*nspcs)
{
    int		spaces = end_col - start_col;
    colnr_T	tabcol = 0;
    int		padding = 0;
    int		tabcount;
    int		t;
    int		ts = ts_arg == 0 ? curbuf->b_p_ts : ts_arg;

    if (vts == NULL || vts[0] == 0)
    {
	int tabs = 0;
	int initspc = 0;

	initspc = ts - (start_col % ts);
	if (spaces >= initspc)
	{
	    spaces -= initspc;
	    tabs++;
	}
	tabs += spaces / ts;
	spaces -= (spaces / ts) * ts;

	*ntabs = tabs;
	*nspcs = spaces;
	return;
    }

    // Find the padding needed to reach the next tabstop.
    tabcount = vts[0];
    for (t = 1; t <= tabcount; ++t)
    {
	tabcol += vts[t];
	if (tabcol > start_col)
	{
	    padding = (int)(tabcol - start_col);
	    break;
	}
    }
    if (t > tabcount)
	padding = vts[tabcount] - (int)((start_col - tabcol) % vts[tabcount]);

    // If the space needed is less than the padding no tabs can be used.
    if (spaces < padding)
    {
	*ntabs = 0;
	*nspcs = spaces;
	return;
    }

    *ntabs = 1;
    spaces -= padding;

    // At least one tab has been used. See if any more will fit.
    while (spaces != 0 && ++t <= tabcount)
    {
	padding = vts[t];
	if (spaces < padding)
	{
	    *nspcs = spaces;
	    return;
	}
	++*ntabs;
	spaces -= padding;
    }

    *ntabs += spaces / vts[tabcount];
    *nspcs =  spaces % vts[tabcount];
}