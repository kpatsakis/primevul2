set_default_colour_sequences(void)
{
    fg_bg_sequences[COL_SEQ_FG].start = ztrdup(TC_COL_FG_START);
    fg_bg_sequences[COL_SEQ_FG].end = ztrdup(TC_COL_FG_END);
    fg_bg_sequences[COL_SEQ_FG].def = ztrdup(TC_COL_FG_DEFAULT);

    fg_bg_sequences[COL_SEQ_BG].start = ztrdup(TC_COL_BG_START);
    fg_bg_sequences[COL_SEQ_BG].end = ztrdup(TC_COL_BG_END);
    fg_bg_sequences[COL_SEQ_BG].def = ztrdup(TC_COL_BG_DEFAULT);
}