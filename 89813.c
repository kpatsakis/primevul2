static void local_socket_event_func(int fd, unsigned ev, void* _s) {
    asocket* s = reinterpret_cast<asocket*>(_s);
    D("LS(%d): event_func(fd=%d(==%d), ev=%04x)", s->id, s->fd, fd, ev);

 /* put the FDE_WRITE processing before the FDE_READ
    ** in order to simplify the code.
    */
 if (ev & FDE_WRITE) {
        apacket* p;
 while ((p = s->pkt_first) != nullptr) {
 while (p->len > 0) {
 int r = adb_write(fd, p->ptr, p->len);
 if (r == -1) {
 /* returning here is ok because FDE_READ will
                    ** be processed in the next iteration loop
                    */
 if (errno == EAGAIN) {
 return;
 }
 } else if (r > 0) {
                    p->ptr += r;
                    p->len -= r;
 continue;
 }

                D(" closing after write because r=%d and errno is %d", r, errno);
                s->has_write_error = true;
                s->close(s);
 return;
 }

 if (p->len == 0) {
                s->pkt_first = p->next;
 if (s->pkt_first == 0) {
                    s->pkt_last = 0;
 }
                put_apacket(p);
 }
 }

 /* if we sent the last packet of a closing socket,
        ** we can now destroy it.
        */
 if (s->closing) {
            D(" closing because 'closing' is set after write");
            s->close(s);
 return;
 }

 /* no more packets queued, so we can ignore
        ** writable events again and tell our peer
        ** to resume writing
        */
        fdevent_del(&s->fde, FDE_WRITE);
        s->peer->ready(s->peer);
 }

 if (ev & FDE_READ) {
        apacket* p = get_apacket();
 unsigned char* x = p->data;
 const size_t max_payload = s->get_max_payload();
 size_t avail = max_payload;
 int r = 0;
 int is_eof = 0;

 while (avail > 0) {
            r = adb_read(fd, x, avail);
            D("LS(%d): post adb_read(fd=%d,...) r=%d (errno=%d) avail=%zu", s->id, s->fd, r,
              r < 0 ? errno : 0, avail);
 if (r == -1) {
 if (errno == EAGAIN) {
 break;
 }
 } else if (r > 0) {
                avail -= r;
                x += r;
 continue;
 }

 /* r = 0 or unhandled error */
            is_eof = 1;
 break;
 }
        D("LS(%d): fd=%d post avail loop. r=%d is_eof=%d forced_eof=%d", s->id, s->fd, r, is_eof,
          s->fde.force_eof);
 if ((avail == max_payload) || (s->peer == 0)) {
            put_apacket(p);
 } else {
            p->len = max_payload - avail;

 unsigned saved_id = s->id;
 int saved_fd = s->fd;
            r = s->peer->enqueue(s->peer, p);
            D("LS(%u): fd=%d post peer->enqueue(). r=%d", saved_id, saved_fd, r);

 if (r < 0) {
 /* error return means they closed us as a side-effect
                ** and we must return immediately.
                **
                ** note that if we still have buffered packets, the
                ** socket will be placed on the closing socket list.
                ** this handler function will be called again
                ** to process FDE_WRITE events.
                */
 return;
 }

 if (r > 0) {
 /* if the remote cannot accept further events,
                ** we disable notification of READs.  They'll
                ** be enabled again when we get a call to ready()
                */
                fdevent_del(&s->fde, FDE_READ);
 }
 }
 /* Don't allow a forced eof if data is still there */
 if ((s->fde.force_eof && !r) || is_eof) {
            D(" closing because is_eof=%d r=%d s->fde.force_eof=%d", is_eof, r, s->fde.force_eof);
            s->close(s);
 return;
 }
 }

 if (ev & FDE_ERROR) {
 /* this should be caught be the next read or write
        ** catching it here means we may skip the last few
        ** bytes of readable data.
        */
        D("LS(%d): FDE_ERROR (fd=%d)", s->id, s->fd);
 return;
 }
}
