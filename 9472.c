static int recv_msg(int sd, char *msg, size_t len, char **cmd, char **argument)
{
	char *ptr;
	ssize_t bytes;
	uint8_t *raw = (uint8_t *)msg;

	/* Clear for every new command. */
	memset(msg, 0, len);

	/* Save one byte (-1) for NUL termination */
	bytes = recv(sd, msg, len - 1, 0);
	if (bytes < 0) {
		if (EINTR == errno)
			return 1;

		if (ECONNRESET == errno)
			DBG("Connection reset by client.");
		else
			ERR(errno, "Failed reading client command");
		return 1;
	}

	if (!bytes) {
		INFO("Client disconnected.");
		return 1;
	}

	if (raw[0] == 0xff) {
		char tmp[4];
		char buf[20] = { 0 };
		int i;

		i = recv(sd, &msg[bytes], len - bytes - 1, MSG_OOB | MSG_DONTWAIT);
		if (i > 0)
			bytes += i;

		for (i = 0; i < bytes; i++) {
			snprintf(tmp, sizeof(tmp), "%2X%s", raw[i], i + 1 < bytes ? " " : "");
			strlcat(buf, tmp, sizeof(buf));
		}

		strlcpy(msg, buf, len);
		*cmd      = msg;
		*argument = NULL;

		DBG("Recv: [%s], %zd bytes", msg, bytes);

		return 0;
	}

	/* NUL terminate for strpbrk() */
	msg[bytes] = 0;

	*cmd = msg;
	ptr  = strpbrk(msg, " ");
	if (ptr) {
		*ptr = 0;
		ptr++;
		*argument = ptr;
	} else {
		*argument = NULL;
		ptr = msg;
	}

	ptr = strpbrk(ptr, "\r\n");
	if (ptr)
		*ptr = 0;

	/* Convert command to std ftp upper case, issue #18 */
	for (ptr = msg; *ptr; ++ptr) *ptr = toupper(*ptr);

	DBG("Recv: %s %s", *cmd, *argument ?: "");

	return 0;
}