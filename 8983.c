allocate_colour_buffer(void)
{
    char **atrs;
    int lenfg, lenbg, len;

    if (colseq_buf_allocs++)
	return;

    atrs = getaparam("zle_highlight");
    if (atrs) {
	for (; *atrs; atrs++) {
	    if (strpfx("fg_start_code:", *atrs)) {
		set_colour_code(*atrs + 14, &fg_bg_sequences[COL_SEQ_FG].start);
	    } else if (strpfx("fg_default_code:", *atrs)) {
		set_colour_code(*atrs + 16, &fg_bg_sequences[COL_SEQ_FG].def);
	    } else if (strpfx("fg_end_code:", *atrs)) {
		set_colour_code(*atrs + 12, &fg_bg_sequences[COL_SEQ_FG].end);
	    } else if (strpfx("bg_start_code:", *atrs)) {
		set_colour_code(*atrs + 14, &fg_bg_sequences[COL_SEQ_BG].start);
	    } else if (strpfx("bg_default_code:", *atrs)) {
		set_colour_code(*atrs + 16, &fg_bg_sequences[COL_SEQ_BG].def);
	    } else if (strpfx("bg_end_code:", *atrs)) {
		set_colour_code(*atrs + 12, &fg_bg_sequences[COL_SEQ_BG].end);
	    }
	}
    }

    lenfg = strlen(fg_bg_sequences[COL_SEQ_FG].def);
    /* always need 1 character for non-default code */
    if (lenfg < 1)
	lenfg = 1;
    lenfg += strlen(fg_bg_sequences[COL_SEQ_FG].start) +
	strlen(fg_bg_sequences[COL_SEQ_FG].end);

    lenbg = strlen(fg_bg_sequences[COL_SEQ_BG].def);
    /* always need 1 character for non-default code */
    if (lenbg < 1)
	lenbg = 1;
    lenbg += strlen(fg_bg_sequences[COL_SEQ_BG].start) +
	strlen(fg_bg_sequences[COL_SEQ_BG].end);

    len = lenfg > lenbg ? lenfg : lenbg;
    /* add 1 for the null and 14 for truecolor */
    colseq_buf = (char *)zalloc(len+15);
}