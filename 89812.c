static int local_socket_enqueue(asocket* s, apacket* p) {
    D("LS(%d): enqueue %d", s->id, p->len);

    p->ptr = p->data;

 /* if there is already data queue'd, we will receive
    ** events when it's time to write.  just add this to
    ** the tail
    */
 if (s->pkt_first) {
 goto enqueue;
 }

 /* write as much as we can, until we
    ** would block or there is an error/eof
    */
 while (p->len > 0) {
 int r = adb_write(s->fd, p->ptr, p->len);
 if (r > 0) {
            p->len -= r;
            p->ptr += r;
 continue;
 }
 if ((r == 0) || (errno != EAGAIN)) {
            D("LS(%d): not ready, errno=%d: %s", s->id, errno, strerror(errno));
            put_apacket(p);
            s->has_write_error = true;
            s->close(s);
 return 1; /* not ready (error) */
 } else {
 break;
 }
 }

 if (p->len == 0) {
        put_apacket(p);
 return 0; /* ready for more data */
 }

enqueue:
    p->next = 0;
 if (s->pkt_first) {
        s->pkt_last->next = p;
 } else {
        s->pkt_first = p;
 }
    s->pkt_last = p;

 /* make sure we are notified when we can drain the queue */
    fdevent_add(&s->fde, FDE_WRITE);

 return 1; /* not ready (backlog) */
}
