int sctp_rcv(struct sk_buff *skb)
{
	struct sock *sk;
	struct sctp_association *asoc;
	struct sctp_endpoint *ep = NULL;
	struct sctp_ep_common *rcvr;
	struct sctp_transport *transport = NULL;
	struct sctp_chunk *chunk;
	struct sctphdr *sh;
	union sctp_addr src;
	union sctp_addr dest;
	int family;
	struct sctp_af *af;

	if (skb->pkt_type!=PACKET_HOST)
		goto discard_it;

	SCTP_INC_STATS_BH(SCTP_MIB_INSCTPPACKS);

	if (skb_linearize(skb))
		goto discard_it;

	sh = sctp_hdr(skb);

	/* Pull up the IP and SCTP headers. */
	__skb_pull(skb, skb_transport_offset(skb));
	if (skb->len < sizeof(struct sctphdr))
		goto discard_it;
	if (!skb_csum_unnecessary(skb) && sctp_rcv_checksum(skb) < 0)
		goto discard_it;

	skb_pull(skb, sizeof(struct sctphdr));

	/* Make sure we at least have chunk headers worth of data left. */
	if (skb->len < sizeof(struct sctp_chunkhdr))
		goto discard_it;

	family = ipver2af(ip_hdr(skb)->version);
	af = sctp_get_af_specific(family);
	if (unlikely(!af))
		goto discard_it;

	/* Initialize local addresses for lookups. */
	af->from_skb(&src, skb, 1);
	af->from_skb(&dest, skb, 0);

	/* If the packet is to or from a non-unicast address,
	 * silently discard the packet.
	 *
	 * This is not clearly defined in the RFC except in section
	 * 8.4 - OOTB handling.  However, based on the book "Stream Control
	 * Transmission Protocol" 2.1, "It is important to note that the
	 * IP address of an SCTP transport address must be a routable
	 * unicast address.  In other words, IP multicast addresses and
	 * IP broadcast addresses cannot be used in an SCTP transport
	 * address."
	 */
	if (!af->addr_valid(&src, NULL, skb) ||
	    !af->addr_valid(&dest, NULL, skb))
		goto discard_it;

	asoc = __sctp_rcv_lookup(skb, &src, &dest, &transport);

	if (!asoc)
		ep = __sctp_rcv_lookup_endpoint(&dest);

	/* Retrieve the common input handling substructure. */
	rcvr = asoc ? &asoc->base : &ep->base;
	sk = rcvr->sk;

	/*
	 * If a frame arrives on an interface and the receiving socket is
	 * bound to another interface, via SO_BINDTODEVICE, treat it as OOTB
	 */
	if (sk->sk_bound_dev_if && (sk->sk_bound_dev_if != af->skb_iif(skb)))
	{
		if (asoc) {
			sctp_association_put(asoc);
			asoc = NULL;
		} else {
			sctp_endpoint_put(ep);
			ep = NULL;
		}
		sk = sctp_get_ctl_sock();
		ep = sctp_sk(sk)->ep;
		sctp_endpoint_hold(ep);
		rcvr = &ep->base;
	}

	/*
	 * RFC 2960, 8.4 - Handle "Out of the blue" Packets.
	 * An SCTP packet is called an "out of the blue" (OOTB)
	 * packet if it is correctly formed, i.e., passed the
	 * receiver's checksum check, but the receiver is not
	 * able to identify the association to which this
	 * packet belongs.
	 */
	if (!asoc) {
		if (sctp_rcv_ootb(skb)) {
			SCTP_INC_STATS_BH(SCTP_MIB_OUTOFBLUES);
			goto discard_release;
		}
	}

	if (!xfrm_policy_check(sk, XFRM_POLICY_IN, skb, family))
		goto discard_release;
	nf_reset(skb);

	if (sk_filter(sk, skb))
		goto discard_release;

	/* Create an SCTP packet structure. */
	chunk = sctp_chunkify(skb, asoc, sk);
	if (!chunk)
		goto discard_release;
	SCTP_INPUT_CB(skb)->chunk = chunk;

	/* Remember what endpoint is to handle this packet. */
	chunk->rcvr = rcvr;

	/* Remember the SCTP header. */
	chunk->sctp_hdr = sh;

	/* Set the source and destination addresses of the incoming chunk.  */
	sctp_init_addrs(chunk, &src, &dest);

	/* Remember where we came from.  */
	chunk->transport = transport;

	/* Acquire access to the sock lock. Note: We are safe from other
	 * bottom halves on this lock, but a user may be in the lock too,
	 * so check if it is busy.
 	 */
 	sctp_bh_lock_sock(sk);
 
	if (sk != rcvr->sk) {
		/* Our cached sk is different from the rcvr->sk.  This is
		 * because migrate()/accept() may have moved the association
		 * to a new socket and released all the sockets.  So now we
		 * are holding a lock on the old socket while the user may
		 * be doing something with the new socket.  Switch our veiw
		 * of the current sk.
		 */
		sctp_bh_unlock_sock(sk);
		sk = rcvr->sk;
		sctp_bh_lock_sock(sk);
	}

 	if (sock_owned_by_user(sk)) {
 		SCTP_INC_STATS_BH(SCTP_MIB_IN_PKT_BACKLOG);
 		sctp_add_backlog(sk, skb);
	} else {
		SCTP_INC_STATS_BH(SCTP_MIB_IN_PKT_SOFTIRQ);
		sctp_inq_push(&chunk->rcvr->inqueue, chunk);
	}

	sctp_bh_unlock_sock(sk);

	/* Release the asoc/ep ref we took in the lookup calls. */
	if (asoc)
		sctp_association_put(asoc);
	else
		sctp_endpoint_put(ep);

	return 0;

discard_it:
	SCTP_INC_STATS_BH(SCTP_MIB_IN_PKT_DISCARDS);
	kfree_skb(skb);
	return 0;

discard_release:
	/* Release the asoc/ep ref we took in the lookup calls. */
	if (asoc)
		sctp_association_put(asoc);
	else
		sctp_endpoint_put(ep);

	goto discard_it;
}
