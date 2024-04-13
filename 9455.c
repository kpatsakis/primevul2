static int open_data_connection(ctrl_t *ctrl)
{
	socklen_t len = sizeof(struct sockaddr);
	struct sockaddr_in sin;

	/* Previous PORT command from client */
	if (ctrl->data_address[0]) {
		int rc;

		ctrl->data_sd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
		if (-1 == ctrl->data_sd) {
			ERR(errno, "Failed creating data socket");
			return -1;
		}

		memset(&sin, 0, sizeof(sin));
		sin.sin_family = AF_INET;
		sin.sin_port = htons(ctrl->data_port);
		inet_aton(ctrl->data_address, &(sin.sin_addr));

		rc = connect(ctrl->data_sd, (struct sockaddr *)&sin, len);
		if (rc == -1 && EINPROGRESS != errno) {
			ERR(errno, "Failed connecting data socket to client");
			close(ctrl->data_sd);
			ctrl->data_sd = -1;

			return -1;
		}

		DBG("Connected successfully to client's previously requested address:PORT %s:%d",
		    ctrl->data_address, ctrl->data_port);
		return 0;
	}

	/* Previous PASV command, accept connect from client */
	if (ctrl->data_listen_sd > 0) {
		const int const_int_1 = 1;
		int retries = 3;
		char client_ip[100];

	retry:
		ctrl->data_sd = accept(ctrl->data_listen_sd, (struct sockaddr *)&sin, &len);
		if (-1 == ctrl->data_sd) {
			if (EAGAIN == errno && --retries) {
				sleep(1);
				goto retry;
			}

			ERR(errno, "Failed accepting connection from client");
			return -1;
		}

		setsockopt(ctrl->data_sd, SOL_SOCKET, SO_KEEPALIVE, &const_int_1, sizeof(const_int_1));
		set_nonblock(ctrl->data_sd);

		inet_ntop(AF_INET, &(sin.sin_addr), client_ip, INET_ADDRSTRLEN);
		DBG("Client PASV data connection from %s:%d", client_ip, ntohs(sin.sin_port));

		close(ctrl->data_listen_sd);
		ctrl->data_listen_sd = -1;
	}

	return 0;
}