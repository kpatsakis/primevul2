main(int argc, char **argv)
{
    int i, c;
    int pid_flags         = 0;
    char *acl             = NULL;
    char *user            = NULL;
    char *password        = NULL;
    char *timeout         = NULL;
    char *method          = NULL;
    char *pid_path        = NULL;
    char *conf_path       = NULL;
    char *iface           = NULL;
    char *manager_address = NULL;
    char *plugin          = NULL;
    char *plugin_opts     = NULL;

    int fast_open  = 0;
    int reuse_port = 0;
    int mode       = TCP_ONLY;
    int mtu        = 0;
    int ipv6first  = 0;

#ifdef HAVE_SETRLIMIT
    static int nofile = 0;
#endif

    int server_num = 0;
    char *server_host[MAX_REMOTE_NUM];

    char *nameservers[MAX_DNS_NUM + 1];
    int nameserver_num = 0;

    jconf_t *conf = NULL;

    static struct option long_options[] = {
        { "fast-open",       no_argument,       NULL, GETOPT_VAL_FAST_OPEN },
        { "reuse-port",      no_argument,       NULL, GETOPT_VAL_REUSE_PORT },
        { "acl",             required_argument, NULL, GETOPT_VAL_ACL },
        { "manager-address", required_argument, NULL,
                                                GETOPT_VAL_MANAGER_ADDRESS },
        { "executable",      required_argument, NULL,
                                                GETOPT_VAL_EXECUTABLE },
        { "mtu",             required_argument, NULL, GETOPT_VAL_MTU },
        { "plugin",          required_argument, NULL, GETOPT_VAL_PLUGIN },
        { "plugin-opts",     required_argument, NULL, GETOPT_VAL_PLUGIN_OPTS },
        { "password",        required_argument, NULL, GETOPT_VAL_PASSWORD },
        { "help",            no_argument,       NULL, GETOPT_VAL_HELP },
        { NULL,              0,                 NULL, 0 }
    };

    opterr = 0;

    USE_TTY();

    while ((c = getopt_long(argc, argv, "f:s:l:k:t:m:c:i:d:a:n:6huUvA",
                            long_options, NULL)) != -1)
        switch (c) {
        case GETOPT_VAL_REUSE_PORT:
            reuse_port = 1;
            break;
        case GETOPT_VAL_FAST_OPEN:
            fast_open = 1;
            break;
        case GETOPT_VAL_ACL:
            acl = optarg;
            break;
        case GETOPT_VAL_MANAGER_ADDRESS:
            manager_address = optarg;
            break;
        case GETOPT_VAL_EXECUTABLE:
            executable = optarg;
            break;
        case GETOPT_VAL_MTU:
            mtu = atoi(optarg);
            break;
        case GETOPT_VAL_PLUGIN:
            plugin = optarg;
            break;
        case GETOPT_VAL_PLUGIN_OPTS:
            plugin_opts = optarg;
            break;
        case 's':
            if (server_num < MAX_REMOTE_NUM) {
                server_host[server_num++] = optarg;
            }
            break;
        case GETOPT_VAL_PASSWORD:
        case 'k':
            password = optarg;
            break;
        case 'f':
            pid_flags = 1;
            pid_path  = optarg;
            break;
        case 't':
            timeout = optarg;
            break;
        case 'm':
            method = optarg;
            break;
        case 'c':
            conf_path = optarg;
            break;
        case 'i':
            iface = optarg;
            break;
        case 'd':
            if (nameserver_num < MAX_DNS_NUM) {
                nameservers[nameserver_num++] = optarg;
            }
            break;
        case 'a':
            user = optarg;
            break;
        case 'u':
            mode = TCP_AND_UDP;
            break;
        case 'U':
            mode = UDP_ONLY;
            break;
        case '6':
            ipv6first = 1;
            break;
        case 'v':
            verbose = 1;
            break;
        case GETOPT_VAL_HELP:
        case 'h':
            usage();
            exit(EXIT_SUCCESS);
#ifdef HAVE_SETRLIMIT
        case 'n':
            nofile = atoi(optarg);
            break;
#endif
        case 'A':
            FATAL("One time auth has been deprecated. Try AEAD ciphers instead.");
            break;
        case '?':
            LOGE("Unrecognized option: %s", optarg);
            opterr = 1;
            break;
        }

    if (opterr) {
        usage();
        exit(EXIT_FAILURE);
    }

    if (conf_path != NULL) {
        conf = read_jconf(conf_path);
        if (server_num == 0) {
            server_num = conf->remote_num;
            for (i = 0; i < server_num; i++)
                server_host[i] = conf->remote_addr[i].host;
        }
        if (password == NULL) {
            password = conf->password;
        }
        if (method == NULL) {
            method = conf->method;
        }
        if (timeout == NULL) {
            timeout = conf->timeout;
        }
        if (user == NULL) {
            user = conf->user;
        }
        if (fast_open == 0) {
            fast_open = conf->fast_open;
        }
        if (reuse_port == 0) {
            reuse_port = conf->reuse_port;
        }
        if (conf->nameserver != NULL) {
            nameservers[nameserver_num++] = conf->nameserver;
        }
        if (mode == TCP_ONLY) {
            mode = conf->mode;
        }
        if (mtu == 0) {
            mtu = conf->mtu;
        }
        if (plugin == NULL) {
            plugin = conf->plugin;
        }
        if (plugin_opts == NULL) {
            plugin_opts = conf->plugin_opts;
        }
        if (ipv6first == 0) {
            ipv6first = conf->ipv6_first;
        }
#ifdef HAVE_SETRLIMIT
        if (nofile == 0) {
            nofile = conf->nofile;
        }
#endif
    }

    if (server_num == 0) {
        server_host[server_num++] = "0.0.0.0";
    }

    if (method == NULL) {
        method = "table";
    }

    if (timeout == NULL) {
        timeout = "60";
    }

    USE_SYSLOG(argv[0], pid_flags);
    if (pid_flags) {
        daemonize(pid_path);
    }

    if (manager_address == NULL) {
        manager_address = "127.0.0.1:8839";
        LOGI("using the default manager address: %s", manager_address);
    }

    if (server_num == 0 || manager_address == NULL) {
        usage();
        exit(EXIT_FAILURE);
    }

    if (fast_open == 1) {
#ifdef TCP_FASTOPEN
        LOGI("using tcp fast open");
#else
        LOGE("tcp fast open is not supported by this environment");
#endif
    }

    signal(SIGPIPE, SIG_IGN);
    signal(SIGCHLD, SIG_IGN);
    signal(SIGABRT, SIG_IGN);

    struct ev_signal sigint_watcher;
    struct ev_signal sigterm_watcher;
    ev_signal_init(&sigint_watcher, signal_cb, SIGINT);
    ev_signal_init(&sigterm_watcher, signal_cb, SIGTERM);
    ev_signal_start(EV_DEFAULT, &sigint_watcher);
    ev_signal_start(EV_DEFAULT, &sigterm_watcher);

    struct manager_ctx manager;
    memset(&manager, 0, sizeof(struct manager_ctx));

    manager.reuse_port      = reuse_port;
    manager.fast_open       = fast_open;
    manager.verbose         = verbose;
    manager.mode            = mode;
    manager.password        = password;
    manager.timeout         = timeout;
    manager.method          = method;
    manager.iface           = iface;
    manager.acl             = acl;
    manager.user            = user;
    manager.manager_address = manager_address;
    manager.hosts           = server_host;
    manager.host_num        = server_num;
    manager.nameservers     = nameservers;
    manager.nameserver_num  = nameserver_num;
    manager.mtu             = mtu;
    manager.plugin          = plugin;
    manager.plugin_opts     = plugin_opts;
    manager.ipv6first       = ipv6first;
#ifdef HAVE_SETRLIMIT
    manager.nofile = nofile;
#endif

    struct ev_loop *loop = EV_DEFAULT;

    if (geteuid() == 0) {
        LOGI("running from root user");
    }

    struct passwd *pw   = getpwuid(getuid());
    const char *homedir = pw->pw_dir;
    working_dir_size = strlen(homedir) + 15;
    working_dir      = ss_malloc(working_dir_size);
    snprintf(working_dir, working_dir_size, "%s/.shadowsocks", homedir);

    int err = mkdir(working_dir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    if (err != 0 && errno != EEXIST) {
        ERROR("mkdir");
        ss_free(working_dir);
        FATAL("unable to create working directory");
    }

    DIR *dp;
    struct dirent *ep;
    dp = opendir(working_dir);
    if (dp != NULL) {
        while ((ep = readdir(dp)) != NULL) {
            size_t len = strlen(ep->d_name);
            if (strcmp(ep->d_name + len - 3, "pid") == 0) {
                kill_server(working_dir, ep->d_name);
                if (verbose)
                    LOGI("kill %s", ep->d_name);
            }
        }
        closedir(dp);
    } else {
        ss_free(working_dir);
        FATAL("Couldn't open the directory");
    }

    server_table = cork_string_hash_table_new(MAX_PORT_NUM, 0);

    if (conf != NULL) {
        for (i = 0; i < conf->port_password_num; i++) {
            struct server *server = ss_malloc(sizeof(struct server));
            memset(server, 0, sizeof(struct server));
            strncpy(server->port, conf->port_password[i].port, 8);
            strncpy(server->password, conf->port_password[i].password, 128);
            add_server(&manager, server);
        }
    }

    int sfd;
    ss_addr_t ip_addr = { .host = NULL, .port = NULL };
    parse_addr(manager_address, &ip_addr);

    if (ip_addr.host == NULL || ip_addr.port == NULL) {
        struct sockaddr_un svaddr;
        sfd = socket(AF_UNIX, SOCK_DGRAM, 0);       /*  Create server socket */
        if (sfd == -1) {
            ss_free(working_dir);
            FATAL("socket");
        }

        setnonblocking(sfd);

        if (remove(manager_address) == -1 && errno != ENOENT) {
            ERROR("bind");
            ss_free(working_dir);
            exit(EXIT_FAILURE);
        }

        memset(&svaddr, 0, sizeof(struct sockaddr_un));
        svaddr.sun_family = AF_UNIX;
        strncpy(svaddr.sun_path, manager_address, sizeof(svaddr.sun_path) - 1);

        if (bind(sfd, (struct sockaddr *)&svaddr, sizeof(struct sockaddr_un)) == -1) {
            ERROR("bind");
            ss_free(working_dir);
            exit(EXIT_FAILURE);
        }
    } else {
        sfd = create_server_socket(ip_addr.host, ip_addr.port);
        if (sfd == -1) {
            ss_free(working_dir);
            FATAL("socket");
        }
    }

    manager.fd = sfd;
    ev_io_init(&manager.io, manager_recv_cb, manager.fd, EV_READ);
    ev_io_start(loop, &manager.io);

    ev_run(loop, 0);

    if (verbose) {
        LOGI("closed gracefully");
    }

    struct cork_hash_table_entry *entry;
    struct cork_hash_table_iterator server_iter;

    cork_hash_table_iterator_init(server_table, &server_iter);

    while ((entry = cork_hash_table_iterator_next(&server_iter)) != NULL) {
        struct server *server = (struct server *)entry->value;
        stop_server(working_dir, server->port);
    }

    ev_signal_stop(EV_DEFAULT, &sigint_watcher);
    ev_signal_stop(EV_DEFAULT, &sigterm_watcher);
    ss_free(working_dir);

    return 0;
}
