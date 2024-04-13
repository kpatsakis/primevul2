match_highlight(const char *teststr, zattr *on_var)
{
    int found = 1;

    *on_var = 0;
    while (found && *teststr) {
	const struct highlight *hl;

	found = 0;
	if (strpfx("fg=", teststr) || strpfx("bg=", teststr)) {
	    int is_fg = (teststr[0] == 'f');
	    zattr atr;

	    teststr += 3;
	    atr = match_colour(&teststr, is_fg, 0);
	    if (*teststr == ',')
		teststr++;
	    else if (*teststr)
		break;
	    found = 1;
	    /* skip out of range colours but keep scanning attributes */
	    if (atr != TXT_ERROR)
		*on_var |= atr;
	} else {
	    for (hl = highlights; hl->name; hl++) {
		if (strpfx(hl->name, teststr)) {
		    const char *val = teststr + strlen(hl->name);

		    if (*val == ',')
			val++;
		    else if (*val)
			break;

		    *on_var |= hl->mask_on;
		    *on_var &= ~hl->mask_off;
		    teststr = val;
		    found = 1;
		}
	    }
	}
    }
}