resolve_pkcs11_token_in_place(char *tokenname)
{
	char *ntn = tokenname;
	size_t pos = 0;

	while (*ntn) {
		char *cp = strchrnul(ntn, ';');
		char c = *cp;
		*cp = '\0';

		dprintf("ntn:\"%s\"", ntn);
		if (!strncmp(&ntn[pos], "token=", 6)) {
			ntn += 6;
			memmove(tokenname, ntn, cp - ntn + 1);
			break;
		}

		*cp = c;
		ntn = cp + (c ? 1 : 0);
	}
	unescape_html_in_place(tokenname);
	dprintf("token name is \"%s\"", tokenname);
}