set_indent(
    int		size,		    // measured in spaces
    int		flags)
{
    char_u	*p;
    char_u	*newline;
    char_u	*oldline;
    char_u	*s;
    int		todo;
    int		ind_len;	    // measured in characters
    int		line_len;
    int		doit = FALSE;
    int		ind_done = 0;	    // measured in spaces
#ifdef FEAT_VARTABS
    int		ind_col = 0;
#endif
    int		tab_pad;
    int		retval = FALSE;
    int		orig_char_len = -1; // number of initial whitespace chars when
				    // 'et' and 'pi' are both set

    // First check if there is anything to do and compute the number of
    // characters needed for the indent.
    todo = size;
    ind_len = 0;
    p = oldline = ml_get_curline();

    // Calculate the buffer size for the new indent, and check to see if it
    // isn't already set

    // if 'expandtab' isn't set: use TABs; if both 'expandtab' and
    // 'preserveindent' are set count the number of characters at the
    // beginning of the line to be copied
    if (!curbuf->b_p_et || (!(flags & SIN_INSERT) && curbuf->b_p_pi))
    {
	// If 'preserveindent' is set then reuse as much as possible of
	// the existing indent structure for the new indent
	if (!(flags & SIN_INSERT) && curbuf->b_p_pi)
	{
	    ind_done = 0;

	    // count as many characters as we can use
	    while (todo > 0 && VIM_ISWHITE(*p))
	    {
		if (*p == TAB)
		{
#ifdef FEAT_VARTABS
		    tab_pad = tabstop_padding(ind_done, curbuf->b_p_ts,
							curbuf->b_p_vts_array);
#else
		    tab_pad = (int)curbuf->b_p_ts
					   - (ind_done % (int)curbuf->b_p_ts);
#endif
		    // stop if this tab will overshoot the target
		    if (todo < tab_pad)
			break;
		    todo -= tab_pad;
		    ++ind_len;
		    ind_done += tab_pad;
		}
		else
		{
		    --todo;
		    ++ind_len;
		    ++ind_done;
		}
		++p;
	    }

#ifdef FEAT_VARTABS
	    // These diverge from this point.
	    ind_col = ind_done;
#endif
	    // Set initial number of whitespace chars to copy if we are
	    // preserving indent but expandtab is set
	    if (curbuf->b_p_et)
		orig_char_len = ind_len;

	    // Fill to next tabstop with a tab, if possible
#ifdef FEAT_VARTABS
	    tab_pad = tabstop_padding(ind_done, curbuf->b_p_ts,
						curbuf->b_p_vts_array);
#else
	    tab_pad = (int)curbuf->b_p_ts - (ind_done % (int)curbuf->b_p_ts);
#endif
	    if (todo >= tab_pad && orig_char_len == -1)
	    {
		doit = TRUE;
		todo -= tab_pad;
		++ind_len;
		// ind_done += tab_pad;
#ifdef FEAT_VARTABS
		ind_col += tab_pad;
#endif
	    }
	}

	// count tabs required for indent
#ifdef FEAT_VARTABS
	for (;;)
	{
	    tab_pad = tabstop_padding(ind_col, curbuf->b_p_ts,
							curbuf->b_p_vts_array);
	    if (todo < tab_pad)
		break;
	    if (*p != TAB)
		doit = TRUE;
	    else
		++p;
	    todo -= tab_pad;
	    ++ind_len;
	    ind_col += tab_pad;
	}
#else
	while (todo >= (int)curbuf->b_p_ts)
	{
	    if (*p != TAB)
		doit = TRUE;
	    else
		++p;
	    todo -= (int)curbuf->b_p_ts;
	    ++ind_len;
	    // ind_done += (int)curbuf->b_p_ts;
	}
#endif
    }
    // count spaces required for indent
    while (todo > 0)
    {
	if (*p != ' ')
	    doit = TRUE;
	else
	    ++p;
	--todo;
	++ind_len;
	// ++ind_done;
    }

    // Return if the indent is OK already.
    if (!doit && !VIM_ISWHITE(*p) && !(flags & SIN_INSERT))
	return FALSE;

    // Allocate memory for the new line.
    if (flags & SIN_INSERT)
	p = oldline;
    else
	p = skipwhite(p);
    line_len = (int)STRLEN(p) + 1;

    // If 'preserveindent' and 'expandtab' are both set keep the original
    // characters and allocate accordingly.  We will fill the rest with spaces
    // after the if (!curbuf->b_p_et) below.
    if (orig_char_len != -1)
    {
	newline = alloc(orig_char_len + size - ind_done + line_len);
	if (newline == NULL)
	    return FALSE;
	todo = size - ind_done;
	ind_len = orig_char_len + todo;    // Set total length of indent in
					   // characters, which may have been
					   // undercounted until now
	p = oldline;
	s = newline;
	while (orig_char_len > 0)
	{
	    *s++ = *p++;
	    orig_char_len--;
	}

	// Skip over any additional white space (useful when newindent is less
	// than old)
	while (VIM_ISWHITE(*p))
	    ++p;

    }
    else
    {
	todo = size;
	newline = alloc(ind_len + line_len);
	if (newline == NULL)
	    return FALSE;
	s = newline;
    }

    // Put the characters in the new line.
    // if 'expandtab' isn't set: use TABs
    if (!curbuf->b_p_et)
    {
	// If 'preserveindent' is set then reuse as much as possible of
	// the existing indent structure for the new indent
	if (!(flags & SIN_INSERT) && curbuf->b_p_pi)
	{
	    p = oldline;
	    ind_done = 0;

	    while (todo > 0 && VIM_ISWHITE(*p))
	    {
		if (*p == TAB)
		{
#ifdef FEAT_VARTABS
		    tab_pad = tabstop_padding(ind_done, curbuf->b_p_ts,
							curbuf->b_p_vts_array);
#else
		    tab_pad = (int)curbuf->b_p_ts
					   - (ind_done % (int)curbuf->b_p_ts);
#endif
		    // stop if this tab will overshoot the target
		    if (todo < tab_pad)
			break;
		    todo -= tab_pad;
		    ind_done += tab_pad;
		}
		else
		{
		    --todo;
		    ++ind_done;
		}
		*s++ = *p++;
	    }

	    // Fill to next tabstop with a tab, if possible
#ifdef FEAT_VARTABS
	    tab_pad = tabstop_padding(ind_done, curbuf->b_p_ts,
						curbuf->b_p_vts_array);
#else
	    tab_pad = (int)curbuf->b_p_ts - (ind_done % (int)curbuf->b_p_ts);
#endif
	    if (todo >= tab_pad)
	    {
		*s++ = TAB;
		todo -= tab_pad;
#ifdef FEAT_VARTABS
		ind_done += tab_pad;
#endif
	    }

	    p = skipwhite(p);
	}

#ifdef FEAT_VARTABS
	for (;;)
	{
	    tab_pad = tabstop_padding(ind_done, curbuf->b_p_ts,
							curbuf->b_p_vts_array);
	    if (todo < tab_pad)
		break;
	    *s++ = TAB;
	    todo -= tab_pad;
	    ind_done += tab_pad;
	}
#else
	while (todo >= (int)curbuf->b_p_ts)
	{
	    *s++ = TAB;
	    todo -= (int)curbuf->b_p_ts;
	}
#endif
    }
    while (todo > 0)
    {
	*s++ = ' ';
	--todo;
    }
    mch_memmove(s, p, (size_t)line_len);

    // Replace the line (unless undo fails).
    if (!(flags & SIN_UNDO) || u_savesub(curwin->w_cursor.lnum) == OK)
    {
	colnr_T old_offset = (colnr_T)(p - oldline);
	colnr_T new_offset = (colnr_T)(s - newline);

	// this may free "newline"
	ml_replace(curwin->w_cursor.lnum, newline, FALSE);
	if (flags & SIN_CHANGED)
	    changed_bytes(curwin->w_cursor.lnum, 0);

	// Correct saved cursor position if it is in this line.
	if (saved_cursor.lnum == curwin->w_cursor.lnum)
	{
	    if (saved_cursor.col >= old_offset)
		// cursor was after the indent, adjust for the number of
		// bytes added/removed
		saved_cursor.col += ind_len - old_offset;
	    else if (saved_cursor.col >= new_offset)
		// cursor was in the indent, and is now after it, put it back
		// at the start of the indent (replacing spaces with TAB)
		saved_cursor.col = new_offset;
	}
#ifdef FEAT_PROP_POPUP
	{
	    int added = ind_len - old_offset;

	    // When increasing indent this behaves like spaces were inserted at
	    // the old indent, when decreasing indent it behaves like spaces
	    // were deleted at the new indent.
	    adjust_prop_columns(curwin->w_cursor.lnum,
			  added > 0 ? old_offset : (colnr_T)ind_len, added, 0);
	}
#endif
	retval = TRUE;
    }
    else
	vim_free(newline);

    curwin->w_cursor.col = ind_len;
    return retval;
}