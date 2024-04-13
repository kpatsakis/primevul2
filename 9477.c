static void do_LIST(uev_t *w, void *arg, int events)
{
	ctrl_t *ctrl = (ctrl_t *)arg;
	struct timeval tv;
	ssize_t bytes;
	char buf[BUFFER_SIZE] = { 0 };

	if (UEV_ERROR == events || UEV_HUP == events) {
		uev_io_start(w);
		return;
	}

	/* Reset inactivity timer. */
	uev_timer_set(&ctrl->timeout_watcher, INACTIVITY_TIMER, 0);

	if (ctrl->d_num == -1) {
		if (ctrl->list_mode == 3)
			do_MLSD(ctrl);
		else
			do_MLST(ctrl);
		do_abort(ctrl);
		return;
	}

	gettimeofday(&tv, NULL);
	if (tv.tv_sec - ctrl->tv.tv_sec > 3) {
		DBG("Sending LIST entry %d of %d to %s ...", ctrl->i, ctrl->d_num, ctrl->clientaddr);
		ctrl->tv.tv_sec = tv.tv_sec;
	}

	ctrl->list_mode |= (ctrl->pending ? 0 : 0x80);
	while (ctrl->i < ctrl->d_num) {
		struct dirent *entry;
		char *name, *path;
		char cwd[PATH_MAX];

		entry = ctrl->d[ctrl->i++];
		name  = entry->d_name;

		DBG("Found directory entry %s", name);
		if ((!strcmp(name, ".") || !strcmp(name, "..")) && ctrl->list_mode < 2)
			continue;

		snprintf(cwd, sizeof(cwd), "%s%s%s", ctrl->file,
			 ctrl->file[strlen(ctrl->file) - 1] == '/' ? "" : "/", name);
		path = compose_path(ctrl, cwd);
		if (!path) {
		fail:
			LOGIT(LOG_INFO, errno, "Failed reading status for %s", path ? path : name);
			continue;
		}

		switch (list_printf(ctrl, buf, sizeof(buf), path, name)) {
		case -1:
			goto fail;
		case 1:
			continue;
		default:
			break;
		}

		DBG("LIST %s", buf);
		free(entry);

		bytes = send(ctrl->data_sd, buf, strlen(buf), 0);
		if (-1 == bytes) {
			if (ECONNRESET == errno)
				DBG("Connection reset by client.");
			else
				ERR(errno, "Failed sending file %s to client", ctrl->file);

			while (ctrl->i < ctrl->d_num) {
				struct dirent *entry = ctrl->d[ctrl->i++];
				free(entry);
			}
			do_abort(ctrl);
			send_msg(ctrl->sd, "426 TCP connection was established but then broken!\r\n");
		}

		return;
	}
	ctrl->list_mode &= 0x0F;

	/* Rewind and list files */
	if (ctrl->pending == 0) {
		ctrl->pending++;
		ctrl->i = 0;
		return;
	}

	do_abort(ctrl);
	send_msg(ctrl->sd, "226 Transfer complete.\r\n");
}