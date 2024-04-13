ex_drop(exarg_T *eap)
{
    int		split = FALSE;
    win_T	*wp;
    buf_T	*buf;
    tabpage_T	*tp;

    if (ERROR_IF_POPUP_WINDOW || ERROR_IF_TERM_POPUP_WINDOW)
	return;

    /*
     * Check if the first argument is already being edited in a window.  If
     * so, jump to that window.
     * We would actually need to check all arguments, but that's complicated
     * and mostly only one file is dropped.
     * This also ignores wildcards, since it is very unlikely the user is
     * editing a file name with a wildcard character.
     */
    set_arglist(eap->arg);

    /*
     * Expanding wildcards may result in an empty argument list.  E.g. when
     * editing "foo.pyc" and ".pyc" is in 'wildignore'.  Assume that we
     * already did an error message for this.
     */
    if (ARGCOUNT == 0)
	return;

    if (cmdmod.cmod_tab)
    {
	// ":tab drop file ...": open a tab for each argument that isn't
	// edited in a window yet.  It's like ":tab all" but without closing
	// windows or tabs.
	ex_all(eap);
    }
    else
    {
	// ":drop file ...": Edit the first argument.  Jump to an existing
	// window if possible, edit in current window if the current buffer
	// can be abandoned, otherwise open a new window.
	buf = buflist_findnr(ARGLIST[0].ae_fnum);

	FOR_ALL_TAB_WINDOWS(tp, wp)
	{
	    if (wp->w_buffer == buf)
	    {
		goto_tabpage_win(tp, wp);
		curwin->w_arg_idx = 0;
		if (!bufIsChanged(curbuf))
		{
		    int save_ar = curbuf->b_p_ar;

		    // reload the file if it is newer
		    curbuf->b_p_ar = TRUE;
		    buf_check_timestamp(curbuf, FALSE);
		    curbuf->b_p_ar = save_ar;
		}
		return;
	    }
	}

	/*
	 * Check whether the current buffer is changed. If so, we will need
	 * to split the current window or data could be lost.
	 * Skip the check if the 'hidden' option is set, as in this case the
	 * buffer won't be lost.
	 */
	if (!buf_hide(curbuf))
	{
	    ++emsg_off;
	    split = check_changed(curbuf, CCGD_AW | CCGD_EXCMD);
	    --emsg_off;
	}

	// Fake a ":sfirst" or ":first" command edit the first argument.
	if (split)
	{
	    eap->cmdidx = CMD_sfirst;
	    eap->cmd[0] = 's';
	}
	else
	    eap->cmdidx = CMD_first;
	ex_rewind(eap);
    }
}