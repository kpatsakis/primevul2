promptexpand(char *s, int ns, char *rs, char *Rs, zattr *txtchangep)
{
    struct buf_vars new_vars;

    if(!s)
	return ztrdup("");

    if ((termflags & TERM_UNKNOWN) && (unset(INTERACTIVE)))
        init_term();

    if (isset(PROMPTSUBST)) {
	int olderr = errflag;
	int oldval = lastval;

	s = dupstring(s);
	if (!parsestr(&s))
	    singsub(&s);
	/*
	 * We don't need the special Nularg hack here and we're
	 * going to be using Nularg for other things.
	 */
	if (*s == Nularg && s[1] == '\0')
	    *s = '\0';

	/*
	 * Ignore errors and status change in prompt substitution.
	 * However, keep any user interrupt error that occurred.
	 */
	errflag = olderr | (errflag & ERRFLAG_INT);
	lastval = oldval;
    }

    memset(&new_vars, 0, sizeof(new_vars));
    new_vars.last = bv;
    bv = &new_vars;

    new_vars.rstring = rs;
    new_vars.Rstring = Rs;
    new_vars.fm = s;
    new_vars.bufspc = 256;
    new_vars.bp = new_vars.bufline = new_vars.buf = zshcalloc(new_vars.bufspc);
    new_vars.bp1 = NULL;
    new_vars.truncwidth = 0;

    putpromptchar(1, '\0', txtchangep);
    addbufspc(2);
    if (new_vars.dontcount)
	*new_vars.bp++ = Outpar;
    *new_vars.bp = '\0';
    if (!ns) {
	/* If zero, Inpar, Outpar and Nularg should be removed. */
	for (new_vars.bp = new_vars.buf; *new_vars.bp; ) {
	    if (*new_vars.bp == Meta)
		new_vars.bp += 2;
	    else if (*new_vars.bp == Inpar || *new_vars.bp == Outpar ||
		     *new_vars.bp == Nularg)
		chuck(new_vars.bp);
	    else
		new_vars.bp++;
	}
    }

    bv = new_vars.last;

    return new_vars.buf;
}