prepare_tagpreview(
    int		undo_sync,	    // sync undo when leaving the window
    int		use_previewpopup,   // use popup if 'previewpopup' set
    use_popup_T	use_popup)	    // use other popup window
{
    win_T	*wp;

# ifdef FEAT_GUI
    need_mouse_correct = TRUE;
# endif

    /*
     * If there is already a preview window open, use that one.
     */
    if (!curwin->w_p_pvw)
    {
# ifdef FEAT_PROP_POPUP
	if (use_previewpopup && *p_pvp != NUL)
	{
	    wp = popup_find_preview_window();
	    if (wp != NULL)
		popup_set_wantpos_cursor(wp, wp->w_minwidth, NULL);
	}
	else if (use_popup != USEPOPUP_NONE)
	{
	    wp = popup_find_info_window();
	    if (wp != NULL)
	    {
		if (use_popup == USEPOPUP_NORMAL)
		    popup_show(wp);
		else
		    popup_hide(wp);
		// When the popup moves or resizes it may reveal part of
		// another window.  TODO: can this be done more efficiently?
		redraw_all_later(NOT_VALID);
	    }
	}
	else
# endif
	{
	    FOR_ALL_WINDOWS(wp)
		if (wp->w_p_pvw)
		    break;
	}
	if (wp != NULL)
	    win_enter(wp, undo_sync);
	else
	{
	    /*
	     * There is no preview window open yet.  Create one.
	     */
# ifdef FEAT_PROP_POPUP
	    if ((use_previewpopup && *p_pvp != NUL)
						 || use_popup != USEPOPUP_NONE)
		return popup_create_preview_window(use_popup != USEPOPUP_NONE);
# endif
	    if (win_split(g_do_tagpreview > 0 ? g_do_tagpreview : 0, 0) == FAIL)
		return FALSE;
	    curwin->w_p_pvw = TRUE;
	    curwin->w_p_wfh = TRUE;
	    RESET_BINDING(curwin);	    // don't take over 'scrollbind'
	    // and 'cursorbind'
# ifdef FEAT_DIFF
	    curwin->w_p_diff = FALSE;	    // no 'diff'
# endif
# ifdef FEAT_FOLDING
	    curwin->w_p_fdc = 0;	    // no 'foldcolumn'
# endif
	    return TRUE;
	}
    }
    return FALSE;
}