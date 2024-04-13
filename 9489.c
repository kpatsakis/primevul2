static void handle_CWD(ctrl_t *ctrl, char *path)
{
	struct stat st;
	char *dir;

	if (!path)
		goto done;

	/*
	 * Some FTP clients, most notably Chrome, use CWD to check if an
	 * entry is a file or directory.
	 */
	dir = compose_abspath(ctrl, path);
	if (!dir || stat(dir, &st) || !S_ISDIR(st.st_mode)) {
		DBG("chrooted:%d, ctrl->cwd: %s, home:%s, dir:%s, len:%zd, dirlen:%zd",
		    chrooted, ctrl->cwd, home, dir, strlen(home), strlen(dir));
		send_msg(ctrl->sd, "550 No such directory.\r\n");
		return;
	}

	if (!chrooted) {
		size_t len = strlen(home);

		DBG("non-chrooted CWD, home:%s, dir:%s, len:%zd, dirlen:%zd",
		    home, dir, len, strlen(dir));
		dir += len;
	}

	snprintf(ctrl->cwd, sizeof(ctrl->cwd), "%s", dir);
	if (ctrl->cwd[0] == 0)
		snprintf(ctrl->cwd, sizeof(ctrl->cwd), "/");

done:
	DBG("New CWD: '%s'", ctrl->cwd);
	send_msg(ctrl->sd, "250 OK\r\n");
}