copy_indent(int size, char_u *src)
{
    char_u	*p = NULL;
    char_u	*line = NULL;
    char_u	*s;
    int		todo;
    int		ind_len;
    int		line_len = 0;
    int		tab_pad;
    int		ind_done;
    int		round;
#ifdef FEAT_VARTABS
    int		ind_col;
#endif

    // Round 1: compute the number of characters needed for the indent
    // Round 2: copy the characters.
    for (round = 1; round <= 2; ++round)
    {
	todo = size;
	ind_len = 0;
	ind_done = 0;
#ifdef FEAT_VARTABS
	ind_col = 0;
#endif
	s = src;

	// Count/copy the usable portion of the source line
	while (todo > 0 && VIM_ISWHITE(*s))
	{
	    if (*s == TAB)
	    {
#ifdef FEAT_VARTABS
		tab_pad = tabstop_padding(ind_done, curbuf->b_p_ts,
							curbuf->b_p_vts_array);
#else
		tab_pad = (int)curbuf->b_p_ts
					   - (ind_done % (int)curbuf->b_p_ts);
#endif
		// Stop if this tab will overshoot the target
		if (todo < tab_pad)
		    break;
		todo -= tab_pad;
		ind_done += tab_pad;
#ifdef FEAT_VARTABS
		ind_col += tab_pad;
#endif
	    }
	    else
	    {
		--todo;
		++ind_done;
#ifdef FEAT_VARTABS
		++ind_col;
#endif
	    }
	    ++ind_len;
	    if (p != NULL)
		*p++ = *s;
	    ++s;
	}

	// Fill to next tabstop with a tab, if possible
#ifdef FEAT_VARTABS
	tab_pad = tabstop_padding(ind_done, curbuf->b_p_ts,
							curbuf->b_p_vts_array);
#else
	tab_pad = (int)curbuf->b_p_ts - (ind_done % (int)curbuf->b_p_ts);
#endif
	if (todo >= tab_pad && !curbuf->b_p_et)
	{
	    todo -= tab_pad;
	    ++ind_len;
#ifdef FEAT_VARTABS
	    ind_col += tab_pad;
#endif
	    if (p != NULL)
		*p++ = TAB;
	}

	// Add tabs required for indent
	if (!curbuf->b_p_et)
	{
#ifdef FEAT_VARTABS
	    for (;;)
	    {
		tab_pad = tabstop_padding(ind_col, curbuf->b_p_ts,
							curbuf->b_p_vts_array);
		if (todo < tab_pad)
		    break;
		todo -= tab_pad;
		++ind_len;
		ind_col += tab_pad;
		if (p != NULL)
		    *p++ = TAB;
	    }
#else
	    while (todo >= (int)curbuf->b_p_ts)
	    {
		todo -= (int)curbuf->b_p_ts;
		++ind_len;
		if (p != NULL)
		    *p++ = TAB;
	    }
#endif
	}

	// Count/add spaces required for indent
	while (todo > 0)
	{
	    --todo;
	    ++ind_len;
	    if (p != NULL)
		*p++ = ' ';
	}

	if (p == NULL)
	{
	    // Allocate memory for the result: the copied indent, new indent
	    // and the rest of the line.
	    line_len = (int)STRLEN(ml_get_curline()) + 1;
	    line = alloc(ind_len + line_len);
	    if (line == NULL)
		return FALSE;
	    p = line;
	}
    }

    // Append the original line
    mch_memmove(p, ml_get_curline(), (size_t)line_len);

    // Replace the line
    ml_replace(curwin->w_cursor.lnum, line, FALSE);

    // Put the cursor after the indent.
    curwin->w_cursor.col = ind_len;
    return TRUE;
}