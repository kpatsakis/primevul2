static void list(ctrl_t *ctrl, char *arg, int mode)
{
	char *path;

	if (string_valid(arg)) {
		char *ptr, *quot;

		/* Check if client sends ls arguments ... */
		ptr = arg;
		while (*ptr) {
			if (isspace(*ptr))
				ptr++;

			if (*ptr == '-') {
				while (*ptr && !isspace(*ptr))
					ptr++;
			}

			break;
		}

		/* Strip any "" from "<arg>" */
		while ((quot = strchr(ptr, '"'))) {
			char *ptr2;

			ptr2 = strchr(&quot[1], '"');
			if (ptr2) {
				memmove(ptr2, &ptr2[1], strlen(ptr2));
				memmove(quot, &quot[1], strlen(quot));
			}
		}
		arg = ptr;
	}

	if (mode >= 2)
		path = compose_abspath(ctrl, arg);
	else
		path = compose_path(ctrl, arg);
	if (!path) {
		send_msg(ctrl->sd, "550 No such file or directory.\r\n");
		return;
	}

	ctrl->list_mode = mode;
	ctrl->file = strdup(arg ? arg : "");
	ctrl->i = 0;
	ctrl->d_num = scandir(path, &ctrl->d, NULL, alphasort);
	if (ctrl->d_num == -1) {
		send_msg(ctrl->sd, "550 No such file or directory.\r\n");
		DBG("Failed reading directory '%s': %s", path, strerror(errno));
		return;
	}

	DBG("Reading directory %s ... %d number of entries", path, ctrl->d_num);
	if (ctrl->data_sd > -1) {
		send_msg(ctrl->sd, "125 Data connection already open; transfer starting.\r\n");
		uev_io_init(ctrl->ctx, &ctrl->data_watcher, do_LIST, ctrl, ctrl->data_sd, UEV_WRITE);
		return;
	}

	do_PORT(ctrl, 1);
}