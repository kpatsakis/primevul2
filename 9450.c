static int list_printf(ctrl_t *ctrl, char *buf, size_t len, char *path, char *name)
{
	int dirs;
	int mode = ctrl->list_mode;
	struct stat st;

	if (stat(path, &st))
		return -1;

	dirs = mode & 0xF0;
	mode = mode & 0x0F;

	if (dirs && !S_ISDIR(st.st_mode))
		return 1;
	if (!dirs && S_ISDIR(st.st_mode))
		return 1;

	switch (mode) {
	case 3:			/* MLSD */
		/* fallthrough */
	case 2:			/* MLST */
		mlsd_printf(ctrl, buf, len, path, name, &st);
		break;

	case 1:			/* NLST */
		snprintf(buf, len, "%s\r\n", name);
		break;

	case 0:			/* LIST */
		snprintf(buf, len, "%s 1 %5d %5d %12" PRIu64 " %s %s\r\n",
			 mode_to_str(st.st_mode),
			 0, 0, (uint64_t)st.st_size,
			 time_to_str(st.st_mtime), name);
		break;
	}

	return 0;
}