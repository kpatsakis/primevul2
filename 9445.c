static void do_PORT(ctrl_t *ctrl, int pending)
{
	if (!ctrl->data_address[0]) {
		/* Check if previous command was PASV */
		if (ctrl->data_sd == -1 && ctrl->data_listen_sd == -1) {
			if (pending == 1 && ctrl->d_num == -1)
				do_MLST(ctrl);
			return;
		}

		ctrl->pending = pending;
		return;
	}

	if (open_data_connection(ctrl)) {
		do_abort(ctrl);
		send_msg(ctrl->sd, "425 TCP connection cannot be established.\r\n");
		return;
	}

	if (pending != 1 || ctrl->list_mode != 2)
		send_msg(ctrl->sd, "150 Data connection opened; transfer starting.\r\n");

	switch (pending) {
	case 3:
		uev_io_init(ctrl->ctx, &ctrl->data_watcher, do_STOR, ctrl, ctrl->data_sd, UEV_READ);
		break;

	case 2:
		uev_io_init(ctrl->ctx, &ctrl->data_watcher, do_RETR, ctrl, ctrl->data_sd, UEV_WRITE);
		break;

	case 1:
		uev_io_init(ctrl->ctx, &ctrl->data_watcher, do_LIST, ctrl, ctrl->data_sd, UEV_WRITE);
		break;
	}

	ctrl->pending = 0;
}