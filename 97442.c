static int find_interface() {
	fd_set read_fds;
	struct mt_packet data;
	struct sockaddr_in myip;
	unsigned char emptymac[ETH_ALEN];
	int testsocket;
	struct timeval timeout;
	int optval = 1;
	struct net_interface *interface;

	/* TODO: reread interfaces on HUP */

	bzero(emptymac, ETH_ALEN);

	if (net_get_interfaces(&interfaces) <= 0) {
		fprintf(stderr, _("Error: No suitable devices found\n"));
		exit(1);
	}

	DL_FOREACH(interfaces, interface) {
		/* Skip loopback interfaces */
		if (memcmp("lo", interface->name, 2) == 0) {
			continue;
		}

		/* Initialize receiving socket on the device chosen */
		myip.sin_family = AF_INET;
		memcpy((void *)&myip.sin_addr, interface->ipv4_addr, IPV4_ALEN);
		myip.sin_port = htons(sourceport);

		/* Initialize socket and bind to udp port */
		if ((testsocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
			continue;
		}

		setsockopt(testsocket, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(optval));
		setsockopt(testsocket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

		if (bind(testsocket, (struct sockaddr *)&myip, sizeof(struct sockaddr_in)) == -1) {
			close(testsocket);
			continue;
		}

		/* Ensure that we have mac-address for this interface  */
		if (!interface->has_mac) {
			close(testsocket);
			continue;
		}

		/* Set the global socket handle and source mac address for send_udp() */
		send_socket = testsocket;
		memcpy(srcmac, interface->mac_addr, ETH_ALEN);
		active_interface = interface;

		/* Send a SESSIONSTART message with the current device */
		init_packet(&data, MT_PTYPE_SESSIONSTART, srcmac, dstmac, sessionkey, 0);
		send_udp(&data, 0);

		timeout.tv_sec = connect_timeout;
		timeout.tv_usec = 0;

		FD_ZERO(&read_fds);
		FD_SET(insockfd, &read_fds);
		select(insockfd + 1, &read_fds, NULL, NULL, &timeout);
		if (FD_ISSET(insockfd, &read_fds)) {
			/* We got a response, this is the correct device to use */
			return 1;
		}

		close(testsocket);
	}
	return 0;
}
