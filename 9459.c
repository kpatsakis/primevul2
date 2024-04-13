static void handle_STOR(ctrl_t *ctrl, char *file)
{
	FILE *fp = NULL;
	char *path;
	int rc = 0;

	path = compose_abspath(ctrl, file);
	if (!path) {
		INFO("Invalid path for %s: %m", file);
		goto fail;
	}

	DBG("Trying to write to %s ...", path);
	fp = fopen(path, "wb");
	if (!fp) {
		/* If EACCESS client is trying to do something disallowed */
		ERR(errno, "Failed writing %s", path);
	fail:
		send_msg(ctrl->sd, "451 Trouble storing file.\r\n");
		do_abort(ctrl);
		return;
	}

	ctrl->fp = fp;
	ctrl->file = strdup(file);

	if (ctrl->data_sd > -1) {
		if (ctrl->offset)
			rc = fseek(fp, ctrl->offset, SEEK_SET);
		if (rc) {
			do_abort(ctrl);
			send_msg(ctrl->sd, "551 Failed seeking to that position in file.\r\n");
			return;
		}

		send_msg(ctrl->sd, "125 Data connection already open; transfer starting.\r\n");
		uev_io_init(ctrl->ctx, &ctrl->data_watcher, do_STOR, ctrl, ctrl->data_sd, UEV_READ);
		return;
	}

	do_PORT(ctrl, 3);
}