struct sctp_chunk *sctp_inq_pop(struct sctp_inq *queue)
{
	struct sctp_chunk *chunk;
	sctp_chunkhdr_t *ch = NULL;

	/* The assumption is that we are safe to process the chunks
	 * at this time.
	 */

	if ((chunk = queue->in_progress)) {
		/* There is a packet that we have been working on.
		 * Any post processing work to do before we move on?
		 */
		if (chunk->singleton ||
		    chunk->end_of_packet ||
		    chunk->pdiscard) {
			sctp_chunk_free(chunk);
			chunk = queue->in_progress = NULL;
 		} else {
 			/* Nothing to do. Next chunk in the packet, please. */
 			ch = (sctp_chunkhdr_t *) chunk->chunk_end;
 			/* Force chunk->skb->data to chunk->chunk_end.  */
			skb_pull(chunk->skb, chunk->chunk_end - chunk->skb->data);
			/* We are guaranteed to pull a SCTP header. */
 		}
 	}
 
	/* Do we need to take the next packet out of the queue to process? */
	if (!chunk) {
		struct list_head *entry;

		/* Is the queue empty?  */
		if (list_empty(&queue->in_chunk_list))
			return NULL;

		entry = queue->in_chunk_list.next;
		chunk = queue->in_progress =
			list_entry(entry, struct sctp_chunk, list);
		list_del_init(entry);

		/* This is the first chunk in the packet.  */
		chunk->singleton = 1;
		ch = (sctp_chunkhdr_t *) chunk->skb->data;
		chunk->data_accepted = 0;
	}

	chunk->chunk_hdr = ch;
	chunk->chunk_end = ((__u8 *)ch) + WORD_ROUND(ntohs(ch->length));
	/* In the unlikely case of an IP reassembly, the skb could be
	 * non-linear. If so, update chunk_end so that it doesn't go past
	 * the skb->tail.
	 */
	if (unlikely(skb_is_nonlinear(chunk->skb))) {
		if (chunk->chunk_end > skb_tail_pointer(chunk->skb))
			chunk->chunk_end = skb_tail_pointer(chunk->skb);
	}
 	skb_pull(chunk->skb, sizeof(sctp_chunkhdr_t));
 	chunk->subh.v = NULL; /* Subheader is no longer valid.  */
 
	if (chunk->chunk_end + sizeof(sctp_chunkhdr_t) <
	    skb_tail_pointer(chunk->skb)) {
 		/* This is not a singleton */
 		chunk->singleton = 0;
 	} else if (chunk->chunk_end > skb_tail_pointer(chunk->skb)) {
		/* Discard inside state machine. */
		chunk->pdiscard = 1;
		chunk->chunk_end = skb_tail_pointer(chunk->skb);
 	} else {
 		/* We are at the end of the packet, so mark the chunk
 		 * in case we need to send a SACK.
		 */
		chunk->end_of_packet = 1;
	}

	pr_debug("+++sctp_inq_pop+++ chunk:%p[%s], length:%d, skb->len:%d\n",
		 chunk, sctp_cname(SCTP_ST_CHUNK(chunk->chunk_hdr->type)),
		 ntohs(chunk->chunk_hdr->length), chunk->skb->len);

	return chunk;
}
