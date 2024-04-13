static void handle_PASV(ctrl_t *ctrl, char *arg)
{
	struct sockaddr_in data;
	socklen_t len = sizeof(data);
	char *msg, *p, buf[200];
	int port;

	if (do_PASV(ctrl, arg, (struct sockaddr *)&data, &len))
		return;

	/* Convert server IP address and port to comma separated list */
	msg = strdup(ctrl->serveraddr);
	if (!msg) {
		send_msg(ctrl->sd, "426 Internal server error.\r\n");
		exit(1);
	}
	p = msg;
	while ((p = strchr(p, '.')))
		*p++ = ',';

	port = ntohs(data.sin_port);
	snprintf(buf, sizeof(buf), "227 Entering Passive Mode (%s,%d,%d)\r\n",
		 msg, port / 256, port % 256);
	send_msg(ctrl->sd, buf);

	free(msg);
}