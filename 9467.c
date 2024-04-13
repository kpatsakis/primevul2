static void handle_MDTM(ctrl_t *ctrl, char *file)
{
	struct stat st;
	struct tm *tm;
	char *path, *ptr;
	char *mtime = NULL;
	char buf[80];

	/* Request to set mtime, ncftp does this */
	ptr = strchr(file, ' ');
	if (ptr) {
		*ptr++ = 0;
		mtime = file;
		file  = ptr;
	}

	path = compose_abspath(ctrl, file);
	if (!path || stat(path, &st) || !S_ISREG(st.st_mode)) {
		send_msg(ctrl->sd, "550 Not a regular file.\r\n");
		return;
	}

	if (mtime) {
		struct timespec times[2] = {
			{ 0, UTIME_OMIT },
			{ 0, 0 }
		};
		struct tm tm;
		int rc;

		if (!strptime(mtime, "%Y%m%d%H%M%S", &tm)) {
		fail:
			send_msg(ctrl->sd, "550 Invalid time format\r\n");
			return;
		}

		times[1].tv_sec = mktime(&tm);
		rc = utimensat(0, path, times, 0);
		if (rc) {
			ERR(errno, "Failed setting MTIME %s of %s", mtime, file);
			goto fail;
		}
		(void)stat(path, &st);
	}

	tm = gmtime(&st.st_mtime);
	strftime(buf, sizeof(buf), "213 %Y%m%d%H%M%S\r\n", tm);

	send_msg(ctrl->sd, buf);
}