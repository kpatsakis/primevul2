int fpm_unix_init_main() /* {{{ */
{
	struct fpm_worker_pool_s *wp;
	int is_root = !geteuid();

	if (fpm_global_config.rlimit_files) {
		struct rlimit r;

		r.rlim_max = r.rlim_cur = (rlim_t) fpm_global_config.rlimit_files;

		if (0 > setrlimit(RLIMIT_NOFILE, &r)) {
			zlog(ZLOG_SYSERROR, "failed to set rlimit_core for this pool. Please check your system limits or decrease rlimit_files. setrlimit(RLIMIT_NOFILE, %d)", fpm_global_config.rlimit_files);
			return -1;
		}
	}

	if (fpm_global_config.rlimit_core) {
		struct rlimit r;

		r.rlim_max = r.rlim_cur = fpm_global_config.rlimit_core == -1 ? (rlim_t) RLIM_INFINITY : (rlim_t) fpm_global_config.rlimit_core;

		if (0 > setrlimit(RLIMIT_CORE, &r)) {
			zlog(ZLOG_SYSERROR, "failed to set rlimit_core for this pool. Please check your system limits or decrease rlimit_core. setrlimit(RLIMIT_CORE, %d)", fpm_global_config.rlimit_core);
			return -1;
		}
	}

	fpm_pagesize = getpagesize();
	if (fpm_global_config.daemonize) {
		/*
		 * If daemonize, the calling process will die soon
		 * and the master process continues to initialize itself.
		 *
		 * The parent process has then to wait for the master
		 * process to initialize to return a consistent exit
		 * value. For this pupose, the master process will
		 * send \"1\" into the pipe if everything went well 
		 * and \"0\" otherwise.
		 */


		struct timeval tv;
		fd_set rfds;
		int ret;

		if (pipe(fpm_globals.send_config_pipe) == -1) {
			zlog(ZLOG_SYSERROR, "failed to create pipe");
			return -1;
		}

		/* then fork */
		pid_t pid = fork();
		switch (pid) {

			case -1 : /* error */
				zlog(ZLOG_SYSERROR, "failed to daemonize");
				return -1;

			case 0 : /* children */
				close(fpm_globals.send_config_pipe[0]); /* close the read side of the pipe */
				break;

			default : /* parent */
				close(fpm_globals.send_config_pipe[1]); /* close the write side of the pipe */

				/*
				 * wait for 10s before exiting with error
				 * the child is supposed to send 1 or 0 into the pipe to tell the parent
				 * how it goes for it
				 */
				FD_ZERO(&rfds);
				FD_SET(fpm_globals.send_config_pipe[0], &rfds);

				tv.tv_sec = 10;
				tv.tv_usec = 0;

				zlog(ZLOG_DEBUG, "The calling process is waiting for the master process to ping via fd=%d", fpm_globals.send_config_pipe[0]);
				ret = select(fpm_globals.send_config_pipe[0] + 1, &rfds, NULL, NULL, &tv);
				if (ret == -1) {
					zlog(ZLOG_SYSERROR, "failed to select");
					exit(FPM_EXIT_SOFTWARE);
				}
				if (ret) { /* data available */
					int readval;
					ret = read(fpm_globals.send_config_pipe[0], &readval, sizeof(readval));
					if (ret == -1) {
						zlog(ZLOG_SYSERROR, "failed to read from pipe");
						exit(FPM_EXIT_SOFTWARE);
					}

					if (ret == 0) {
						zlog(ZLOG_ERROR, "no data have been read from pipe");
						exit(FPM_EXIT_SOFTWARE);
					} else {
						if (readval == 1) {
							zlog(ZLOG_DEBUG, "I received a valid acknoledge from the master process, I can exit without error");
							fpm_cleanups_run(FPM_CLEANUP_PARENT_EXIT);
							exit(FPM_EXIT_OK);
						} else {
							zlog(ZLOG_DEBUG, "The master process returned an error !");
							exit(FPM_EXIT_SOFTWARE);
						}
					}
				} else { /* no date sent ! */
					zlog(ZLOG_ERROR, "the master process didn't send back its status (via the pipe to the calling process)");
				  exit(FPM_EXIT_SOFTWARE);
				}
				exit(FPM_EXIT_SOFTWARE);
		}
	}

	/* continue as a child */
	setsid();
	if (0 > fpm_clock_init()) {
		return -1;
	}

	if (fpm_global_config.process_priority != 64) {
		if (is_root) {
			if (setpriority(PRIO_PROCESS, 0, fpm_global_config.process_priority) < 0) {
				zlog(ZLOG_SYSERROR, "Unable to set priority for the master process");
				return -1;
			}
		} else {
			zlog(ZLOG_NOTICE, "'process.priority' directive is ignored when FPM is not running as root");
		}
	}

	fpm_globals.parent_pid = getpid();
	for (wp = fpm_worker_all_pools; wp; wp = wp->next) {
		if (0 > fpm_unix_conf_wp(wp)) {
			return -1;
		}
	}

	zlog_set_level(fpm_globals.log_level);
	return 0;
}
/* }}} */
