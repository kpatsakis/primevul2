static void read_client_command(uev_t *w, void *arg, int events)
{
	char *command, *argument;
	ctrl_t *ctrl = (ctrl_t *)arg;
	ftp_cmd_t *cmd;

	if (UEV_ERROR == events || UEV_HUP == events) {
		uev_io_start(w);
		return;
	}

	/* Reset inactivity timer. */
	uev_timer_set(&ctrl->timeout_watcher, INACTIVITY_TIMER, 0);

	if (recv_msg(w->fd, ctrl->buf, ctrl->bufsz, &command, &argument)) {
		DBG("Short read, exiting.");
		uev_exit(ctrl->ctx);
		return;
	}

	if (!string_valid(command))
		return;

	if (string_match(command, "FF F4")) {
		DBG("Ignoring IAC command, client should send ABOR as well.");
		return;
	}

	for (cmd = &supported[0]; cmd->command; cmd++) {
		if (string_compare(command, cmd->command)) {
			cmd->cb(ctrl, argument);
			return;
		}
	}

	handle_UNKNOWN(ctrl, command);
}