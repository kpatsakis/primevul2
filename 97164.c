int append_serve(const SERVER *const s, GArray *const a) {
	SERVER *ns = NULL;
	struct addrinfo hints;
	struct addrinfo *ai = NULL;
	struct addrinfo *rp = NULL;
	char   host[NI_MAXHOST];
	gchar  *port = NULL;
	int e;
	int ret;

	assert(s != NULL);

	port = g_strdup_printf("%d", s->port);

	memset(&hints,'\0',sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_ADDRCONFIG | AI_PASSIVE;
	hints.ai_protocol = IPPROTO_TCP;

	e = getaddrinfo(s->listenaddr, port, &hints, &ai);

	if (port)
		g_free(port);

	if(e == 0) {
		for (rp = ai; rp != NULL; rp = rp->ai_next) {
			e = getnameinfo(rp->ai_addr, rp->ai_addrlen, host, sizeof(host), NULL, 0, NI_NUMERICHOST);

			if (e != 0) { // error
				fprintf(stderr, "getnameinfo: %s\n", gai_strerror(e));
				continue;
			}

			ns = dup_serve (s);
			if (ns) {
				ns->listenaddr = g_strdup(host);
				ns->socket_family = rp->ai_family;
				g_array_append_val(a, *ns);
				free(ns);
				ns = NULL;
			}
		}

		ret = 0;
	} else {
		fprintf(stderr, "getaddrinfo failed on listen host/address: %s (%s)\n", s->listenaddr ? s->listenaddr : "any", gai_strerror(e));
		ret = -1;
	}

	if (ai)
		freeaddrinfo(ai);

	return ret;
}
