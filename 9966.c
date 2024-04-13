get_indent_str(
    char_u	*ptr,
    int		ts,
    int		list) // if TRUE, count only screen size for tabs
{
    int		count = 0;

    for ( ; *ptr; ++ptr)
    {
	if (*ptr == TAB)
	{
	    if (!list || curwin->w_lcs_chars.tab1)
		// count a tab for what it is worth
		count += ts - (count % ts);
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