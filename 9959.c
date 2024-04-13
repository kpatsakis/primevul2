get_indent_str_vtab(char_u *ptr, int ts, int *vts, int list)
{
    int		count = 0;

    for ( ; *ptr; ++ptr)
    {
	if (*ptr == TAB)    // count a tab for what it is worth
	{
	    if (!list || curwin->w_lcs_chars.tab1)
		count += tabstop_padding(count, ts, vts);
	    else
		// In list mode, when tab is not set, count screen char width
		// for Tab, displays: ^I
		count += ptr2cells(ptr);
	}
	else if (*ptr == ' ')
	    ++count;		// count a space for one
	else
	    break;
    }
    return count;
}