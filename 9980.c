ex_oldfiles(exarg_T *eap UNUSED)
{
    list_T	*l = get_vim_var_list(VV_OLDFILES);
    listitem_T	*li;
    int		nr = 0;
    char_u	*fname;

    if (l == NULL)
	msg(_("No old files"));
    else
    {
	msg_start();
	msg_scroll = TRUE;
	for (li = l->lv_first; li != NULL && !got_int; li = li->li_next)
	{
	    ++nr;
	    fname = tv_get_string(&li->li_tv);
	    if (!message_filtered(fname))
	    {
		msg_outnum((long)nr);
		msg_puts(": ");
		msg_outtrans(fname);
		msg_clr_eos();
		msg_putchar('\n');
		out_flush();	    // output one line at a time
		ui_breakcheck();
	    }
	}

	// Assume "got_int" was set to truncate the listing.
	got_int = FALSE;

# ifdef FEAT_BROWSE_CMD
	if (cmdmod.cmod_flags & CMOD_BROWSE)
	{
	    quit_more = FALSE;
	    nr = prompt_for_number(FALSE);
	    msg_starthere();
	    if (nr > 0)
	    {
		char_u *p = list_find_str(get_vim_var_list(VV_OLDFILES),
								    (long)nr);

		if (p != NULL)
		{
		    p = expand_env_save(p);
		    eap->arg = p;
		    eap->cmdidx = CMD_edit;
		    cmdmod.cmod_flags &= ~CMOD_BROWSE;
		    do_exedit(eap, NULL);
		    vim_free(p);
		}
	    }
	}
# endif
    }
}