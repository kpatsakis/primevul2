static unsigned int ssh_tty_parse_boolean(char *s)
{
    if (stricmp(s, "yes") == 0 ||
	stricmp(s, "on") == 0 ||
	stricmp(s, "true") == 0 ||
	stricmp(s, "+") == 0)
	return 1; /* true */
    else if (stricmp(s, "no") == 0 ||
	     stricmp(s, "off") == 0 ||
	     stricmp(s, "false") == 0 ||
	     stricmp(s, "-") == 0)
	return 0; /* false */
    else
	return (atoi(s) != 0);
}
