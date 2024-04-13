static void do_pasv_connection(uev_t *w, void *arg, int events)
{
	ctrl_t *ctrl = (ctrl_t *)arg;
	int rc = 0;

	if (UEV_ERROR == events || UEV_HUP == events) {
		DBG("error on data_listen_sd ...");
		uev_io_start(w);
		return;
	}
	DBG("Event on data_listen_sd ...");
	uev_io_stop(&ctrl->data_watcher);
	if (open_data_connection(ctrl))
		return;

	switch (ctrl->pending) {
	case 3:
		/* fall-through */
	case 2:
		if (ctrl->offset)
			rc = fseek(ctrl->fp, ctrl->offset, SEEK_SET);
		if (rc) {
			do_abort(ctrl);
			send_msg(ctrl->sd, "551 Failed seeking to that position in file.\r\n");
			return;
		}
		/* fall-through */
	case 1:
		break;

	default:
		DBG("No pending command, waiting ...");
		return;
	}

	switch (ctrl->pending) {
	case 3:			/* STOR */
		DBG("Pending STOR, starting ...");
		uev_io_init(ctrl->ctx, &ctrl->data_watcher, do_STOR, ctrl, ctrl->data_sd, UEV_READ);
		break;

	case 2:			/* RETR */
		DBG("Pending RETR, starting ...");
		uev_io_init(ctrl->ctx, &ctrl->data_watcher, do_RETR, ctrl, ctrl->data_sd, UEV_WRITE);
		break;

	case 1:			/* LIST */
		DBG("Pending LIST, starting ...");
		uev_io_init(ctrl->ctx, &ctrl->data_watcher, do_LIST, ctrl, ctrl->data_sd, UEV_WRITE);
		break;
	}

	if (ctrl->pending == 1 && ctrl->list_mode == 2)
		send_msg(ctrl->sd, "150 Opening ASCII mode data connection for MLSD.\r\n");
	else
		send_msg(ctrl->sd, "150 Data connection accepted; transfer starting.\r\n");
	ctrl->pending = 0;
}