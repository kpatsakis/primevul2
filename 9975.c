fixthisline(int (*get_the_indent)(void))
{
    int amount = get_the_indent();

    if (amount >= 0)
    {
	change_indent(INDENT_SET, amount, FALSE, 0, TRUE);
	if (linewhite(curwin->w_cursor.lnum))
	    did_ai = TRUE;	// delete the indent if the line stays empty
    }
}