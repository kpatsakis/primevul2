make_filter_cmd(
    char_u	*cmd,		// command
    char_u	*itmp,		// NULL or name of input file
    char_u	*otmp)		// NULL or name of output file
{
    char_u	*buf;
    long_u	len;

#if defined(UNIX)
    int		is_fish_shell;
    char_u	*shell_name = get_isolated_shell_name();

    if (shell_name == NULL)
	return NULL;

    // Account for fish's different syntax for subshells
    is_fish_shell = (fnamecmp(shell_name, "fish") == 0);
    vim_free(shell_name);
    if (is_fish_shell)
	len = (long_u)STRLEN(cmd) + 13;		// "begin; " + "; end" + NUL
    else
#endif
	len = (long_u)STRLEN(cmd) + 3;			// "()" + NUL
    if (itmp != NULL)
	len += (long_u)STRLEN(itmp) + 9;		// " { < " + " } "
    if (otmp != NULL)
	len += (long_u)STRLEN(otmp) + (long_u)STRLEN(p_srr) + 2; // "  "
    buf = alloc(len);
    if (buf == NULL)
	return NULL;

#if defined(UNIX)
    /*
     * Put braces around the command (for concatenated commands) when
     * redirecting input and/or output.
     */
    if (itmp != NULL || otmp != NULL)
    {
	if (is_fish_shell)
	    vim_snprintf((char *)buf, len, "begin; %s; end", (char *)cmd);
	else
	    vim_snprintf((char *)buf, len, "(%s)", (char *)cmd);
    }
    else
	STRCPY(buf, cmd);
    if (itmp != NULL)
    {
	STRCAT(buf, " < ");
	STRCAT(buf, itmp);
    }
#else
    // For shells that don't understand braces around commands, at least allow
    // the use of commands in a pipe.
    if (*p_sxe != NUL && *p_sxq == '(')
    {
	if (itmp != NULL || otmp != NULL)
	    vim_snprintf((char *)buf, len, "(%s)", (char *)cmd);
	else
	    STRCPY(buf, cmd);
	if (itmp != NULL)
	{
	    STRCAT(buf, " < ");
	    STRCAT(buf, itmp);
	}
    }
    else
    {
	STRCPY(buf, cmd);
	if (itmp != NULL)
	{
	    char_u	*p;

	    // If there is a pipe, we have to put the '<' in front of it.
	    // Don't do this when 'shellquote' is not empty, otherwise the
	    // redirection would be inside the quotes.
	    if (*p_shq == NUL)
	    {
		p = find_pipe(buf);
		if (p != NULL)
		    *p = NUL;
	    }
	    STRCAT(buf, " <");	// " < " causes problems on Amiga
	    STRCAT(buf, itmp);
	    if (*p_shq == NUL)
	    {
		p = find_pipe(cmd);
		if (p != NULL)
		{
		    STRCAT(buf, " ");  // insert a space before the '|' for DOS
		    STRCAT(buf, p);
		}
	    }
	}
    }
#endif
    if (otmp != NULL)
	append_redir(buf, (int)len, p_srr, otmp);

    return buf;
}