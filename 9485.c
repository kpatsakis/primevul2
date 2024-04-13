static void do_STOR(uev_t *w, void *arg, int events)
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
		DBG("no fp for STOR, bailing.");
		return;
	}

	/* Reset inactivity timer. */
	uev_timer_set(&ctrl->timeout_watcher, INACTIVITY_TIMER, 0);

	bytes = recv(ctrl->data_sd, buf, sizeof(buf), 0);
	if (bytes < 0) {
		if (ECONNRESET == errno)
			DBG("Connection reset by client.");
		else
			ERR(errno, "Failed receiving file %s from client", ctrl->file);
		do_abort(ctrl);
		send_msg(ctrl->sd, "426 TCP connection was established but then broken!\r\n");
		return;
	}
	if (bytes == 0) {
		INFO("User %s at %s uploaded file %s", ctrl->name, ctrl->clientaddr, ctrl->file);
		do_abort(ctrl);
		send_msg(ctrl->sd, "226 Transfer complete.\r\n");
		return;
	}

	gettimeofday(&tv, NULL);
	if (tv.tv_sec - ctrl->tv.tv_sec > 3) {
		DBG("Receiving %zd bytes of %s from %s ...", bytes, ctrl->file, ctrl->clientaddr);
		ctrl->tv.tv_sec = tv.tv_sec;
	}

	num = fwrite(buf, 1, bytes, ctrl->fp);
	if ((size_t)bytes != num)
		ERR(errno, "552 Disk full.");
}