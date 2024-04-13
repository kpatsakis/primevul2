match_named_colour(const char **teststrp)
{
    const char *teststr = *teststrp, *end, **cptr;
    int len;

    for (end = teststr; ialpha(*end); end++)
	;
    len = end - teststr;
    *teststrp = end;

    for (cptr = ansi_colours; *cptr; cptr++) {
	if (!strncmp(teststr, *cptr, len))
	    return cptr - ansi_colours;
    }

    return -1;
}