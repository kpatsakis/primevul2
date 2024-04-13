skip_substitute(char_u *start, int delimiter)
{
    char_u *p = start;

    while (p[0])
    {
	if (p[0] == delimiter)		// end delimiter found
	{
	    *p++ = NUL;			// replace it with a NUL
	    break;
	}
	if (p[0] == '\\' && p[1] != 0)	// skip escaped characters
	    ++p;
	MB_PTR_ADV(p);
    }
    return p;
}