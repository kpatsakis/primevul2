 handle_connection(GArray *servers, int net, SERVER *serve, CLIENT *client)
 {
	int sock_flags_old;
	int sock_flags_new;

	if(serve->max_connections > 0 &&
	   g_hash_table_size(children) >= serve->max_connections) {
		msg(LOG_INFO, "Max connections reached");
		goto handle_connection_out;
	}
	if((sock_flags_old = fcntl(net, F_GETFL, 0)) == -1) {
		err("fcntl F_GETFL");
	}
	sock_flags_new = sock_flags_old & ~O_NONBLOCK;
	if (sock_flags_new != sock_flags_old &&
	    fcntl(net, F_SETFL, sock_flags_new) == -1) {
		err("fcntl F_SETFL ~O_NONBLOCK");
	}
	if(!client) {
		client = g_new0(CLIENT, 1);
		client->server=serve;
		client->exportsize=OFFT_MAX;
		client->net=net;
		client->transactionlogfd = -1;
	}
	if (set_peername(net, client)) {
		goto handle_connection_out;
	}
	if (!authorized_client(client)) {
		msg(LOG_INFO, "Unauthorized client");
		goto handle_connection_out;
	}
	msg(LOG_INFO, "Authorized client");

	if (!dontfork) {
		pid_t pid;
		int i;
		sigset_t newset;
		sigset_t oldset;

		sigemptyset(&newset);
		sigaddset(&newset, SIGCHLD);
		sigaddset(&newset, SIGTERM);
		sigprocmask(SIG_BLOCK, &newset, &oldset);
		if ((pid = fork()) < 0) {
			msg(LOG_INFO, "Could not fork (%s)", strerror(errno));
			sigprocmask(SIG_SETMASK, &oldset, NULL);
			goto handle_connection_out;
		}
		if (pid > 0) { /* parent */
			pid_t *pidp;

			pidp = g_malloc(sizeof(pid_t));
			*pidp = pid;
			g_hash_table_insert(children, pidp, pidp);
			sigprocmask(SIG_SETMASK, &oldset, NULL);
			goto handle_connection_out;
		}
		/* child */
		signal(SIGCHLD, SIG_DFL);
		signal(SIGTERM, SIG_DFL);
		signal(SIGHUP, SIG_DFL);
		sigprocmask(SIG_SETMASK, &oldset, NULL);

		g_hash_table_destroy(children);
		children = NULL;
		for(i=0;i<servers->len;i++) {
			serve=&g_array_index(servers, SERVER, i);
			close(serve->socket);
		}
		/* FALSE does not free the
		   actual data. This is required,
		   because the client has a
		   direct reference into that
		   data, and otherwise we get a
		   segfault... */
		g_array_free(servers, FALSE);
		for(i=0;i<modernsocks->len;i++) {
			close(g_array_index(modernsocks, int, i));
		}
		g_array_free(modernsocks, TRUE);
	}

	msg(LOG_INFO, "Starting to serve");
	serveconnection(client);
	exit(EXIT_SUCCESS);

handle_connection_out:
	g_free(client);
	close(net);
}
