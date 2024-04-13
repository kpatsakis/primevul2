static int close_data_connection(ctrl_t *ctrl)
{
	int ret = 0;

	DBG("Closing data connection ...");

	/* PASV server listening socket */
	if (ctrl->data_listen_sd > 0) {
		shutdown(ctrl->data_listen_sd, SHUT_RDWR);
		close(ctrl->data_listen_sd);
		ctrl->data_listen_sd = -1;
		ret++;
	}

	/* PASV client socket */
	if (ctrl->data_sd > 0) {
		shutdown(ctrl->data_sd, SHUT_RDWR);
		close(ctrl->data_sd);
		ctrl->data_sd = -1;
		ret++;
	}

	/* PORT */
	if (ctrl->data_address[0]) {
		ctrl->data_address[0] = 0;
		ctrl->data_port = 0;
	}

	return ret;
}