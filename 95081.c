static void ssh_socket_log(Plug plug, int type, SockAddr addr, int port,
                           const char *error_msg, int error_code)
{
    Ssh ssh = (Ssh) plug;

    /*
     * While we're attempting connection sharing, don't loudly log
     * everything that happens. Real TCP connections need to be logged
     * when we _start_ trying to connect, because it might be ages
     * before they respond if something goes wrong; but connection
     * sharing is local and quick to respond, and it's sufficient to
     * simply wait and see whether it worked afterwards.
     */

    if (!ssh->attempting_connshare)
        backend_socket_log(ssh->frontend, type, addr, port,
                           error_msg, error_code, ssh->conf,
                           ssh->session_started);
}
