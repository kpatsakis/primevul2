static int smart_socket_enqueue(asocket* s, apacket* p) {
 unsigned len;
#if ADB_HOST
 char* service = nullptr;
 char* serial = nullptr;
 TransportType type = kTransportAny;
#endif

    D("SS(%d): enqueue %d", s->id, p->len);

 if (s->pkt_first == 0) {
        s->pkt_first = p;
        s->pkt_last = p;
 } else {
 if ((s->pkt_first->len + p->len) > s->get_max_payload()) {
            D("SS(%d): overflow", s->id);
            put_apacket(p);
 goto fail;
 }

        memcpy(s->pkt_first->data + s->pkt_first->len, p->data, p->len);
        s->pkt_first->len += p->len;
        put_apacket(p);

        p = s->pkt_first;
 }

 /* don't bother if we can't decode the length */
 if (p->len < 4) {
 return 0;
 }

    len = unhex(p->data, 4);
 if ((len < 1) || (len > MAX_PAYLOAD_V1)) {
        D("SS(%d): bad size (%d)", s->id, len);
 goto fail;
 }

    D("SS(%d): len is %d", s->id, len);
 /* can't do anything until we have the full header */
 if ((len + 4) > p->len) {
        D("SS(%d): waiting for %d more bytes", s->id, len + 4 - p->len);
 return 0;
 }

    p->data[len + 4] = 0;

    D("SS(%d): '%s'", s->id, (char*)(p->data + 4));

#if ADB_HOST
    service = (char*)p->data + 4;
 if (!strncmp(service, "host-serial:", strlen("host-serial:"))) {
 char* serial_end;
        service += strlen("host-serial:");

        serial_end = internal::skip_host_serial(service);
 if (serial_end) {
 *serial_end = 0; // terminate string
            serial = service;
            service = serial_end + 1;
 }
 } else if (!strncmp(service, "host-usb:", strlen("host-usb:"))) {
        type = kTransportUsb;
        service += strlen("host-usb:");
 } else if (!strncmp(service, "host-local:", strlen("host-local:"))) {
        type = kTransportLocal;
        service += strlen("host-local:");
 } else if (!strncmp(service, "host:", strlen("host:"))) {
        type = kTransportAny;
        service += strlen("host:");
 } else {
        service = nullptr;
 }

 if (service) {
        asocket* s2;

 /* some requests are handled immediately -- in that
        ** case the handle_host_request() routine has sent
        ** the OKAY or FAIL message and all we have to do
        ** is clean up.
        */
 if (handle_host_request(service, type, serial, s->peer->fd, s) == 0) {
 /* XXX fail message? */
            D("SS(%d): handled host service '%s'", s->id, service);
 goto fail;
 }
 if (!strncmp(service, "transport", strlen("transport"))) {
            D("SS(%d): okay transport", s->id);
            p->len = 0;
 return 0;
 }

 /* try to find a local service with this name.
        ** if no such service exists, we'll fail out
        ** and tear down here.
        */
        s2 = create_host_service_socket(service, serial);
 if (s2 == 0) {
            D("SS(%d): couldn't create host service '%s'", s->id, service);
 SendFail(s->peer->fd, "unknown host service");
 goto fail;
 }

 /* we've connected to a local host service,
        ** so we make our peer back into a regular
        ** local socket and bind it to the new local
        ** service socket, acknowledge the successful
        ** connection, and close this smart socket now
        ** that its work is done.
        */
 SendOkay(s->peer->fd);

        s->peer->ready = local_socket_ready;
        s->peer->shutdown = nullptr;
        s->peer->close = local_socket_close;
        s->peer->peer = s2;
        s2->peer = s->peer;
        s->peer = 0;
        D("SS(%d): okay", s->id);
        s->close(s);

 /* initial state is "ready" */
        s2->ready(s2);
 return 0;
 }
#else /* !ADB_HOST */
 if (s->transport == nullptr) {
        std::string error_msg = "unknown failure";
        s->transport = acquire_one_transport(kTransportAny, nullptr, nullptr, &error_msg);
 if (s->transport == nullptr) {
 SendFail(s->peer->fd, error_msg);
 goto fail;
 }
 }
#endif

 if (!(s->transport) || (s->transport->connection_state == kCsOffline)) {
 /* if there's no remote we fail the connection
         ** right here and terminate it
         */
 SendFail(s->peer->fd, "device offline (x)");
 goto fail;
 }

 /* instrument our peer to pass the success or fail
    ** message back once it connects or closes, then
    ** detach from it, request the connection, and
    ** tear down
    */
    s->peer->ready = local_socket_ready_notify;
    s->peer->shutdown = nullptr;
    s->peer->close = local_socket_close_notify;
    s->peer->peer = 0;
 /* give him our transport and upref it */
    s->peer->transport = s->transport;

    connect_to_remote(s->peer, (char*)(p->data + 4));
    s->peer = 0;
    s->close(s);
 return 1;

fail:
 /* we're going to close our peer as a side-effect, so
    ** return -1 to signal that state to the local socket
    ** who is enqueueing against us
    */
    s->close(s);
 return -1;
}
