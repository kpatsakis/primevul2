global_exe(char_u *cmd)
{
    linenr_T old_lcount;	// b_ml.ml_line_count before the command
    buf_T    *old_buf = curbuf;	// remember what buffer we started in
    linenr_T lnum;		// line number according to old situation

    /*
     * Set current position only once for a global command.
     * If global_busy is set, setpcmark() will not do anything.
     * If there is an error, global_busy will be incremented.
     */
    setpcmark();

    // When the command writes a message, don't overwrite the command.
    msg_didout = TRUE;

    sub_nsubs = 0;
    sub_nlines = 0;
    global_need_beginline = FALSE;
    global_busy = 1;
    old_lcount = curbuf->b_ml.ml_line_count;
    while (!got_int && (lnum = ml_firstmarked()) != 0 && global_busy == 1)
    {
	global_exe_one(cmd, lnum);
	ui_breakcheck();
    }

    global_busy = 0;
    if (global_need_beginline)
	beginline(BL_WHITE | BL_FIX);
    else
	check_cursor();	// cursor may be beyond the end of the line

    // the cursor may not have moved in the text but a change in a previous
    // line may move it on the screen
    changed_line_abv_curs();

    // If it looks like no message was written, allow overwriting the
    // command with the report for number of changes.
    if (msg_col == 0 && msg_scrolled == 0)
	msg_didout = FALSE;

    // If substitutes done, report number of substitutes, otherwise report
    // number of extra or deleted lines.
    // Don't report extra or deleted lines in the edge case where the buffer
    // we are in after execution is different from the buffer we started in.
    if (!do_sub_msg(FALSE) && curbuf == old_buf)
	msgmore(curbuf->b_ml.ml_line_count - old_lcount);
}