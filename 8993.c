promptpath(char *p, int npath, int tilde)
{
    char *modp = p;
    Nameddir nd;

    if (tilde && ((nd = finddir(p))))
	modp = tricat("~", nd->node.nam, p + strlen(nd->dir));

    if (npath) {
	char *sptr;
	if (npath > 0) {
	    for (sptr = modp + strlen(modp); sptr > modp; sptr--) {
		if (*sptr == '/' && !--npath) {
		    sptr++;
		    break;
		}
	    }
	    if (*sptr == '/' && sptr[1] && sptr != modp)
		sptr++;
	    stradd(sptr);
	} else {
	    char cbu;
	    for (sptr = modp+1; *sptr; sptr++)
		if (*sptr == '/' && !++npath)
		    break;
	    cbu = *sptr;
	    *sptr = 0;
	    stradd(modp);
	    *sptr = cbu;
	}
    } else
	stradd(modp);

    if (p != modp)
	zsfree(modp);
}