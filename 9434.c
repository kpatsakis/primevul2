static void mlsd_printf(ctrl_t *ctrl, char *buf, size_t len, char *path, char *name, struct stat *st)
{
	char perms[10] = "";
	int ro = !access(path, R_OK);
	int rw = !access(path, W_OK);

	if (S_ISDIR(st->st_mode)) {
		/* XXX: Verify 'e' by checking that we can CD to the 'name' */
		if (ro)
			strlcat(perms, "le", sizeof(perms));
		if (rw)
			strlcat(perms, "pc", sizeof(perms)); /* 'd' RMD, 'm' MKD */
	} else {
		if (ro)
			strlcat(perms, "r", sizeof(perms));
		if (rw)
			strlcat(perms, "w", sizeof(perms)); /* 'f' RNFR, 'd' DELE */
	}

	memset(buf, 0, len);
	if (ctrl->d_num == -1 && (ctrl->list_mode & 0x0F) == 2)
		strlcat(buf, " ", len);

	for (int i = 0; ctrl->facts[i]; i++)
		mlsd_fact(ctrl->facts[i], buf, len, name, perms, st);

	strlcat(buf, " ", len);
	strlcat(buf, name, len);
	strlcat(buf, "\r\n", len);
}