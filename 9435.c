static void do_RETR(uev_t *w, void *arg, int events)
{
	ctrl_t *ctrl = (ctrl_t *)arg;
	struct timeval tv;
	ssize_t bytes;
	size_t num;
	char buf[BUFFER_SIZE];

	if (UEV_ERROR == events || UEV_HUP == events) {
		DBG("error on data_sd ...");
		uev_io_start(w);
		return;
	}

	if (!ctrl->fp) {
		DBG("no fp for RETR, bailing.");
		return;
	}

	num = fread(buf, sizeof(char), sizeof(buf), ctrl->fp);
	if (!num) {
		if (feof(ctrl->fp))
			INFO("User %s from %s downloaded %s", ctrl->name, ctrl->clientaddr, ctrl->file);
		else if (ferror(ctrl->fp))
			ERR(0, "Error while reading %s", ctrl->file);
		do_abort(ctrl);
		send_msg(ctrl->sd, "226 Transfer complete.\r\n");
		return;
	}

	/* Reset inactivity timer. */
	uev_timer_set(&ctrl->timeout_watcher, INACTIVITY_TIMER, 0);

	gettimeofday(&tv, NULL);
	if (tv.tv_sec - ctrl->tv.tv_sec > 3) {
		DBG("Sending %zd bytes of %s to %s ...", num, ctrl->file, ctrl->clientaddr);
		ctrl->tv.tv_sec = tv.tv_sec;
	}

	bytes = send(ctrl->data_sd, buf, num, 0);
	if (-1 == bytes) {
		if (ECONNRESET == errno)
			DBG("Connection reset by client.");
		else
			ERR(errno, "Failed sending file %s to client", ctrl->file);

		do_abort(ctrl);
		send_msg(ctrl->sd, "426 TCP connection was established but then broken!\r\n");
	}
}