detect_form(struct archive_read *a, int *is_form_d)
{
	const char *p;
	ssize_t avail, ravail;
	ssize_t detected_bytes = 0, len, nl;
	int entry_cnt = 0, multiline = 0;
	int form_D = 0;/* The archive is generated by `NetBSD mtree -D'
			* (In this source we call it `form D') . */

	if (is_form_d != NULL)
		*is_form_d = 0;
	p = __archive_read_ahead(a, 1, &avail);
	if (p == NULL)
		return (-1);
	ravail = avail;
	for (;;) {
		len = next_line(a, &p, &avail, &ravail, &nl);
		/* The terminal character of the line should be
		 * a new line character, '\r\n' or '\n'. */
		if (len <= 0 || nl == 0)
			break;
		if (!multiline) {
			/* Leading whitespace is never significant,
			 * ignore it. */
			while (len > 0 && (*p == ' ' || *p == '\t')) {
				++p;
				--avail;
				--len;
			}
			/* Skip comment or empty line. */ 
			if (p[0] == '#' || p[0] == '\n' || p[0] == '\r') {
				p += len;
				avail -= len;
				continue;
			}
		} else {
			/* A continuance line; the terminal
			 * character of previous line was '\' character. */
			if (bid_keyword_list(p, len, 0, 0) <= 0)
				break;
			if (multiline == 1)
				detected_bytes += len;
			if (p[len-nl-1] != '\\') {
				if (multiline == 1 &&
				    ++entry_cnt >= MAX_BID_ENTRY)
					break;
				multiline = 0;
			}
			p += len;
			avail -= len;
			continue;
		}
		if (p[0] != '/') {
			int last_is_path, keywords;

			keywords = bid_entry(p, len, nl, &last_is_path);
			if (keywords >= 0) {
				detected_bytes += len;
				if (form_D == 0) {
					if (last_is_path)
						form_D = 1;
					else if (keywords > 0)
						/* This line is not `form D'. */
						form_D = -1;
				} else if (form_D == 1) {
					if (!last_is_path && keywords > 0)
						/* This this is not `form D'
						 * and We cannot accept mixed
						 * format. */
						break;
				}
				if (!last_is_path && p[len-nl-1] == '\\')
					/* This line continues. */
					multiline = 1;
				else {
					/* We've got plenty of correct lines
					 * to assume that this file is a mtree
					 * format. */
					if (++entry_cnt >= MAX_BID_ENTRY)
						break;
				}
			} else
				break;
		} else if (strncmp(p, "/set", 4) == 0) {
			if (bid_keyword_list(p+4, len-4, 0, 0) <= 0)
				break;
			/* This line continues. */
			if (p[len-nl-1] == '\\')
				multiline = 2;
		} else if (strncmp(p, "/unset", 6) == 0) {
			if (bid_keyword_list(p+6, len-6, 1, 0) <= 0)
				break;
			/* This line continues. */
			if (p[len-nl-1] == '\\')
				multiline = 2;
		} else
			break;

		/* Test next line. */
		p += len;
		avail -= len;
	}
	if (entry_cnt >= MAX_BID_ENTRY || (entry_cnt > 0 && len == 0)) {
		if (is_form_d != NULL) {
			if (form_D == 1)
				*is_form_d = 1;
		}
		return (32);
	}

	return (0);
}