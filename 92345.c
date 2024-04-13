int dtls1_process_buffered_records(SSL *s)
 {
     pitem *item;
     SSL3_BUFFER *rb;
    SSL3_RECORD *rr;
    DTLS1_BITMAP *bitmap;
    unsigned int is_next_epoch;
    int replayok = 1;
 
     item = pqueue_peek(s->rlayer.d->unprocessed_rcds.q);
     if (item) {
         /* Check if epoch is current. */
         if (s->rlayer.d->unprocessed_rcds.epoch != s->rlayer.d->r_epoch)
            return 1;         /* Nothing to do. */

        rr = RECORD_LAYER_get_rrec(&s->rlayer);
 
         rb = RECORD_LAYER_get_rbuf(&s->rlayer);
 
             */
            return 1;
        }

        /* Process all the records. */
        while (pqueue_peek(s->rlayer.d->unprocessed_rcds.q)) {
            dtls1_get_unprocessed_record(s);
            if (!dtls1_process_record(s))
                return (0);
            if (dtls1_buffer_record(s, &(s->rlayer.d->processed_rcds),
         /* Process all the records. */
         while (pqueue_peek(s->rlayer.d->unprocessed_rcds.q)) {
             dtls1_get_unprocessed_record(s);
            bitmap = dtls1_get_bitmap(s, rr, &is_next_epoch);
            if (bitmap == NULL) {
                /*
                 * Should not happen. This will only ever be NULL when the
                 * current record is from a different epoch. But that cannot
                 * be the case because we already checked the epoch above
                 */
                 SSLerr(SSL_F_DTLS1_PROCESS_BUFFERED_RECORDS,
                        ERR_R_INTERNAL_ERROR);
                 return 0;
            }
#ifndef OPENSSL_NO_SCTP
            /* Only do replay check if no SCTP bio */
            if (!BIO_dgram_is_sctp(SSL_get_rbio(s)))
#endif
            {
                /*
                 * Check whether this is a repeat, or aged record. We did this
                 * check once already when we first received the record - but
                 * we might have updated the window since then due to
                 * records we subsequently processed.
                 */
                replayok = dtls1_record_replay_check(s, bitmap);
            }

            if (!replayok || !dtls1_process_record(s, bitmap)) {
                /* dump this record */
                rr->length = 0;
                RECORD_LAYER_reset_packet_length(&s->rlayer);
                continue;
            }

             if (dtls1_buffer_record(s, &(s->rlayer.d->processed_rcds),
                    SSL3_RECORD_get_seq_num(s->rlayer.rrec)) < 0)
                return 0;
         }
     }
 
 *             here, anything else is handled by higher layers
 *     Application data protocol
 *             none of our business
 */
     s->rlayer.d->processed_rcds.epoch = s->rlayer.d->r_epoch;
     s->rlayer.d->unprocessed_rcds.epoch = s->rlayer.d->r_epoch + 1;
 
    return 1;
 }
