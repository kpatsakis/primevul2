append_redir(
    char_u	*buf,
    int		buflen,
    char_u	*opt,
    char_u	*fname)
{
    char_u	*p;
    char_u	*end;

    end = buf + STRLEN(buf);
    // find "%s"
    for (p = opt; (p = vim_strchr(p, '%')) != NULL; ++p)
    {
	if (p[1] == 's') // found %s
	    break;
	if (p[1] == '%') // skip %%
	    ++p;
    }
    if (p != NULL)
    {
#ifdef MSWIN
	*end++ = ' '; // not really needed? Not with sh, ksh or bash
#endif
	vim_snprintf((char *)end, (size_t)(buflen - (end - buf)),
						  (char *)opt, (char *)fname);
    }
    else
	vim_snprintf((char *)end, (size_t)(buflen - (end - buf)),
#ifdef FEAT_QUICKFIX
		" %s %s",
#else
		" %s%s",	// " > %s" causes problems on Amiga
#endif
		(char *)opt, (char *)fname);
}