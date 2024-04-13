unescape_html_in_place(char *s)
{
	size_t sz = strlen(s) + 1;
	size_t pos = 0;
	char *s1;

	dprintf("unescaping pos:%zd sz:%zd \"%s\"", pos, sz, s);
	do {
		s1 = strchrnul(&s[pos], '%');
		if (s1[0] == '\0')
			break;
		dprintf("s1 is \"%s\"", s1);
		if ((size_t)(s1 - s) < (size_t)(sz - 3)) {
			int c;

			c = (hexchar_to_bin(s1[1]) << 4)
			    | (hexchar_to_bin(s1[2]) & 0xf);
			dprintf("replacing %%%c%c with 0x%02hhx", s1[1], s1[2], (char)c);
			s1[0] = c;
			memmove(&s1[1], &s1[3], sz - (&s1[3] - s));
			sz -= 2;
			pos = &s1[1] - s;
			dprintf("new pos:%zd sz:%zd s:\"%s\"", pos, sz, s);
		}
	} while (pos < sz);
}