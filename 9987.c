ex_file(exarg_T *eap)
{
    // ":0file" removes the file name.  Check for illegal uses ":3file",
    // "0file name", etc.
    if (eap->addr_count > 0
	    && (*eap->arg != NUL
		|| eap->line2 > 0
		|| eap->addr_count > 1))
    {
	emsg(_(e_invalid_argument));
	return;
    }

    if (*eap->arg != NUL || eap->addr_count == 1)
    {
	if (rename_buffer(eap->arg) == FAIL)
	    return;
	redraw_tabline = TRUE;
    }

    // print file name if no argument or 'F' is not in 'shortmess'
    if (*eap->arg == NUL || !shortmess(SHM_FILEINFO))
	fileinfo(FALSE, FALSE, eap->forceit);
}