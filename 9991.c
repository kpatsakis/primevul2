do_write(exarg_T *eap)
{
    int		other;
    char_u	*fname = NULL;		// init to shut up gcc
    char_u	*ffname;
    int		retval = FAIL;
    char_u	*free_fname = NULL;
#ifdef FEAT_BROWSE
    char_u	*browse_file = NULL;
#endif
    buf_T	*alt_buf = NULL;
    int		name_was_missing;

    if (not_writing())		// check 'write' option
	return FAIL;

    ffname = eap->arg;
#ifdef FEAT_BROWSE
    if ((cmdmod.cmod_flags & CMOD_BROWSE) && !exiting)
    {
	browse_file = do_browse(BROWSE_SAVE, (char_u *)_("Save As"), ffname,
						    NULL, NULL, NULL, curbuf);
	if (browse_file == NULL)
	    goto theend;
	ffname = browse_file;
    }
#endif
    if (*ffname == NUL)
    {
	if (eap->cmdidx == CMD_saveas)
	{
	    emsg(_(e_argument_required));
	    goto theend;
	}
	other = FALSE;
    }
    else
    {
	fname = ffname;
	free_fname = fix_fname(ffname);
	/*
	 * When out-of-memory, keep unexpanded file name, because we MUST be
	 * able to write the file in this situation.
	 */
	if (free_fname != NULL)
	    ffname = free_fname;
	other = otherfile(ffname);
    }

    /*
     * If we have a new file, put its name in the list of alternate file names.
     */
    if (other)
    {
	if (vim_strchr(p_cpo, CPO_ALTWRITE) != NULL
						 || eap->cmdidx == CMD_saveas)
	    alt_buf = setaltfname(ffname, fname, (linenr_T)1);
	else
	    alt_buf = buflist_findname(ffname);
	if (alt_buf != NULL && alt_buf->b_ml.ml_mfp != NULL)
	{
	    // Overwriting a file that is loaded in another buffer is not a
	    // good idea.
	    emsg(_(e_file_is_loaded_in_another_buffer));
	    goto theend;
	}
    }

    /*
     * Writing to the current file is not allowed in readonly mode
     * and a file name is required.
     * "nofile" and "nowrite" buffers cannot be written implicitly either.
     */
    if (!other && (
#ifdef FEAT_QUICKFIX
		bt_dontwrite_msg(curbuf) ||
#endif
		check_fname() == FAIL
#ifdef UNIX
		|| check_writable(curbuf->b_ffname) == FAIL
#endif
		|| check_readonly(&eap->forceit, curbuf)))
	goto theend;

    if (!other)
    {
	ffname = curbuf->b_ffname;
	fname = curbuf->b_fname;
	/*
	 * Not writing the whole file is only allowed with '!'.
	 */
	if (	   (eap->line1 != 1
		    || eap->line2 != curbuf->b_ml.ml_line_count)
		&& !eap->forceit
		&& !eap->append
		&& !p_wa)
	{
#if defined(FEAT_GUI_DIALOG) || defined(FEAT_CON_DIALOG)
	    if (p_confirm || (cmdmod.cmod_flags & CMOD_CONFIRM))
	    {
		if (vim_dialog_yesno(VIM_QUESTION, NULL,
			       (char_u *)_("Write partial file?"), 2) != VIM_YES)
		    goto theend;
		eap->forceit = TRUE;
	    }
	    else
#endif
	    {
		emsg(_(e_use_bang_to_write_partial_buffer));
		goto theend;
	    }
	}
    }

    if (check_overwrite(eap, curbuf, fname, ffname, other) == OK)
    {
	if (eap->cmdidx == CMD_saveas && alt_buf != NULL)
	{
	    buf_T	*was_curbuf = curbuf;

	    apply_autocmds(EVENT_BUFFILEPRE, NULL, NULL, FALSE, curbuf);
	    apply_autocmds(EVENT_BUFFILEPRE, NULL, NULL, FALSE, alt_buf);
#ifdef FEAT_EVAL
	    if (curbuf != was_curbuf || aborting())
#else
	    if (curbuf != was_curbuf)
#endif
	    {
		// buffer changed, don't change name now
		retval = FAIL;
		goto theend;
	    }
	    // Exchange the file names for the current and the alternate
	    // buffer.  This makes it look like we are now editing the buffer
	    // under the new name.  Must be done before buf_write(), because
	    // if there is no file name and 'cpo' contains 'F', it will set
	    // the file name.
	    fname = alt_buf->b_fname;
	    alt_buf->b_fname = curbuf->b_fname;
	    curbuf->b_fname = fname;
	    fname = alt_buf->b_ffname;
	    alt_buf->b_ffname = curbuf->b_ffname;
	    curbuf->b_ffname = fname;
	    fname = alt_buf->b_sfname;
	    alt_buf->b_sfname = curbuf->b_sfname;
	    curbuf->b_sfname = fname;
	    buf_name_changed(curbuf);

	    apply_autocmds(EVENT_BUFFILEPOST, NULL, NULL, FALSE, curbuf);
	    apply_autocmds(EVENT_BUFFILEPOST, NULL, NULL, FALSE, alt_buf);
	    if (!alt_buf->b_p_bl)
	    {
		alt_buf->b_p_bl = TRUE;
		apply_autocmds(EVENT_BUFADD, NULL, NULL, FALSE, alt_buf);
	    }
#ifdef FEAT_EVAL
	    if (curbuf != was_curbuf || aborting())
#else
	    if (curbuf != was_curbuf)
#endif
	    {
		// buffer changed, don't write the file
		retval = FAIL;
		goto theend;
	    }

	    // If 'filetype' was empty try detecting it now.
	    if (*curbuf->b_p_ft == NUL)
	    {
		if (au_has_group((char_u *)"filetypedetect"))
		    (void)do_doautocmd((char_u *)"filetypedetect BufRead",
								  TRUE, NULL);
		do_modelines(0);
	    }

	    // Autocommands may have changed buffer names, esp. when
	    // 'autochdir' is set.
	    fname = curbuf->b_sfname;
	}

	name_was_missing = curbuf->b_ffname == NULL;

	retval = buf_write(curbuf, ffname, fname, eap->line1, eap->line2,
				 eap, eap->append, eap->forceit, TRUE, FALSE);

	// After ":saveas fname" reset 'readonly'.
	if (eap->cmdidx == CMD_saveas)
	{
	    if (retval == OK)
	    {
		curbuf->b_p_ro = FALSE;
		redraw_tabline = TRUE;
	    }
	}

	// Change directories when the 'acd' option is set and the file name
	// got changed or set.
	if (eap->cmdidx == CMD_saveas || name_was_missing)
	    DO_AUTOCHDIR;
    }

theend:
#ifdef FEAT_BROWSE
    vim_free(browse_file);
#endif
    vim_free(free_fname);
    return retval;
}