static int __ip6_append_data(struct sock *sk,
			     struct flowi6 *fl6,
			     struct sk_buff_head *queue,
			     struct inet_cork *cork,
			     struct inet6_cork *v6_cork,
			     struct page_frag *pfrag,
			     int getfrag(void *from, char *to, int offset,
					 int len, int odd, struct sk_buff *skb),
			     void *from, int length, int transhdrlen,
			     unsigned int flags, struct ipcm6_cookie *ipc6,
			     const struct sockcm_cookie *sockc)
{
	struct sk_buff *skb, *skb_prev = NULL;
	unsigned int maxfraglen, fragheaderlen, mtu, orig_mtu;
	int exthdrlen = 0;
	int dst_exthdrlen = 0;
	int hh_len;
	int copy;
	int err;
	int offset = 0;
	__u8 tx_flags = 0;
	u32 tskey = 0;
	struct rt6_info *rt = (struct rt6_info *)cork->dst;
	struct ipv6_txoptions *opt = v6_cork->opt;
	int csummode = CHECKSUM_NONE;
	unsigned int maxnonfragsize, headersize;

	skb = skb_peek_tail(queue);
	if (!skb) {
		exthdrlen = opt ? opt->opt_flen : 0;
		dst_exthdrlen = rt->dst.header_len - rt->rt6i_nfheader_len;
	}

	mtu = cork->fragsize;
	orig_mtu = mtu;

	hh_len = LL_RESERVED_SPACE(rt->dst.dev);

	fragheaderlen = sizeof(struct ipv6hdr) + rt->rt6i_nfheader_len +
			(opt ? opt->opt_nflen : 0);
	maxfraglen = ((mtu - fragheaderlen) & ~7) + fragheaderlen -
		     sizeof(struct frag_hdr);

	headersize = sizeof(struct ipv6hdr) +
		     (opt ? opt->opt_flen + opt->opt_nflen : 0) +
		     (dst_allfrag(&rt->dst) ?
		      sizeof(struct frag_hdr) : 0) +
		     rt->rt6i_nfheader_len;

	if (cork->length + length > mtu - headersize && ipc6->dontfrag &&
	    (sk->sk_protocol == IPPROTO_UDP ||
	     sk->sk_protocol == IPPROTO_RAW)) {
		ipv6_local_rxpmtu(sk, fl6, mtu - headersize +
				sizeof(struct ipv6hdr));
		goto emsgsize;
	}

	if (ip6_sk_ignore_df(sk))
		maxnonfragsize = sizeof(struct ipv6hdr) + IPV6_MAXPLEN;
	else
		maxnonfragsize = mtu;

	if (cork->length + length > maxnonfragsize - headersize) {
emsgsize:
		ipv6_local_error(sk, EMSGSIZE, fl6,
				 mtu - headersize +
				 sizeof(struct ipv6hdr));
		return -EMSGSIZE;
	}

	/* CHECKSUM_PARTIAL only with no extension headers and when
	 * we are not going to fragment
	 */
	if (transhdrlen && sk->sk_protocol == IPPROTO_UDP &&
	    headersize == sizeof(struct ipv6hdr) &&
	    length <= mtu - headersize &&
	    !(flags & MSG_MORE) &&
	    rt->dst.dev->features & (NETIF_F_IPV6_CSUM | NETIF_F_HW_CSUM))
		csummode = CHECKSUM_PARTIAL;

	if (sk->sk_type == SOCK_DGRAM || sk->sk_type == SOCK_RAW) {
		sock_tx_timestamp(sk, sockc->tsflags, &tx_flags);
		if (tx_flags & SKBTX_ANY_SW_TSTAMP &&
		    sk->sk_tsflags & SOF_TIMESTAMPING_OPT_ID)
			tskey = sk->sk_tskey++;
	}

	/*
	 * Let's try using as much space as possible.
	 * Use MTU if total length of the message fits into the MTU.
	 * Otherwise, we need to reserve fragment header and
	 * fragment alignment (= 8-15 octects, in total).
	 *
	 * Note that we may need to "move" the data from the tail of
	 * of the buffer to the new fragment when we split
	 * the message.
	 *
	 * FIXME: It may be fragmented into multiple chunks
	 *        at once if non-fragmentable extension headers
	 *        are too large.
	 * --yoshfuji
	 */

	cork->length += length;
	if ((((length + fragheaderlen) > mtu) ||
	     (skb && skb_is_gso(skb))) &&
	    (sk->sk_protocol == IPPROTO_UDP) &&
	    (rt->dst.dev->features & NETIF_F_UFO) && !dst_xfrm(&rt->dst) &&
	    (sk->sk_type == SOCK_DGRAM) && !udp_get_no_check6_tx(sk)) {
		err = ip6_ufo_append_data(sk, queue, getfrag, from, length,
					  hh_len, fragheaderlen, exthdrlen,
					  transhdrlen, mtu, flags, fl6);
		if (err)
			goto error;
		return 0;
	}

	if (!skb)
		goto alloc_new_skb;

	while (length > 0) {
		/* Check if the remaining data fits into current packet. */
		copy = (cork->length <= mtu && !(cork->flags & IPCORK_ALLFRAG) ? mtu : maxfraglen) - skb->len;
		if (copy < length)
			copy = maxfraglen - skb->len;

		if (copy <= 0) {
			char *data;
			unsigned int datalen;
			unsigned int fraglen;
			unsigned int fraggap;
			unsigned int alloclen;
alloc_new_skb:
			/* There's no room in the current skb */
			if (skb)
				fraggap = skb->len - maxfraglen;
			else
				fraggap = 0;
			/* update mtu and maxfraglen if necessary */
			if (!skb || !skb_prev)
				ip6_append_data_mtu(&mtu, &maxfraglen,
						    fragheaderlen, skb, rt,
						    orig_mtu);

			skb_prev = skb;

			/*
			 * If remaining data exceeds the mtu,
			 * we know we need more fragment(s).
			 */
			datalen = length + fraggap;

			if (datalen > (cork->length <= mtu && !(cork->flags & IPCORK_ALLFRAG) ? mtu : maxfraglen) - fragheaderlen)
				datalen = maxfraglen - fragheaderlen - rt->dst.trailer_len;
			if ((flags & MSG_MORE) &&
			    !(rt->dst.dev->features&NETIF_F_SG))
				alloclen = mtu;
			else
				alloclen = datalen + fragheaderlen;

			alloclen += dst_exthdrlen;

			if (datalen != length + fraggap) {
				/*
				 * this is not the last fragment, the trailer
				 * space is regarded as data space.
				 */
				datalen += rt->dst.trailer_len;
			}

			alloclen += rt->dst.trailer_len;
			fraglen = datalen + fragheaderlen;

			/*
			 * We just reserve space for fragment header.
			 * Note: this may be overallocation if the message
			 * (without MSG_MORE) fits into the MTU.
 			 */
 			alloclen += sizeof(struct frag_hdr);
 
			copy = datalen - transhdrlen - fraggap;
			if (copy < 0) {
				err = -EINVAL;
				goto error;
			}
 			if (transhdrlen) {
 				skb = sock_alloc_send_skb(sk,
 						alloclen + hh_len,
						(flags & MSG_DONTWAIT), &err);
			} else {
				skb = NULL;
				if (atomic_read(&sk->sk_wmem_alloc) <=
				    2 * sk->sk_sndbuf)
					skb = sock_wmalloc(sk,
							   alloclen + hh_len, 1,
							   sk->sk_allocation);
				if (unlikely(!skb))
					err = -ENOBUFS;
			}
			if (!skb)
				goto error;
			/*
			 *	Fill in the control structures
			 */
			skb->protocol = htons(ETH_P_IPV6);
			skb->ip_summed = csummode;
			skb->csum = 0;
			/* reserve for fragmentation and ipsec header */
			skb_reserve(skb, hh_len + sizeof(struct frag_hdr) +
				    dst_exthdrlen);

			/* Only the initial fragment is time stamped */
			skb_shinfo(skb)->tx_flags = tx_flags;
			tx_flags = 0;
			skb_shinfo(skb)->tskey = tskey;
			tskey = 0;

			/*
			 *	Find where to start putting bytes
			 */
			data = skb_put(skb, fraglen);
			skb_set_network_header(skb, exthdrlen);
			data += fragheaderlen;
			skb->transport_header = (skb->network_header +
						 fragheaderlen);
			if (fraggap) {
				skb->csum = skb_copy_and_csum_bits(
					skb_prev, maxfraglen,
					data + transhdrlen, fraggap, 0);
				skb_prev->csum = csum_sub(skb_prev->csum,
							  skb->csum);
 				data += fraggap;
 				pskb_trim_unique(skb_prev, maxfraglen);
 			}
			if (copy > 0 &&
			    getfrag(from, data + transhdrlen, offset,
				    copy, fraggap, skb) < 0) {
 				err = -EFAULT;
 				kfree_skb(skb);
 				goto error;
			}

			offset += copy;
			length -= datalen - fraggap;
			transhdrlen = 0;
			exthdrlen = 0;
			dst_exthdrlen = 0;

			if ((flags & MSG_CONFIRM) && !skb_prev)
				skb_set_dst_pending_confirm(skb, 1);

			/*
			 * Put the packet on the pending queue
			 */
			__skb_queue_tail(queue, skb);
			continue;
		}

		if (copy > length)
			copy = length;

		if (!(rt->dst.dev->features&NETIF_F_SG)) {
			unsigned int off;

			off = skb->len;
			if (getfrag(from, skb_put(skb, copy),
						offset, copy, off, skb) < 0) {
				__skb_trim(skb, off);
				err = -EFAULT;
				goto error;
			}
		} else {
			int i = skb_shinfo(skb)->nr_frags;

			err = -ENOMEM;
			if (!sk_page_frag_refill(sk, pfrag))
				goto error;

			if (!skb_can_coalesce(skb, i, pfrag->page,
					      pfrag->offset)) {
				err = -EMSGSIZE;
				if (i == MAX_SKB_FRAGS)
					goto error;

				__skb_fill_page_desc(skb, i, pfrag->page,
						     pfrag->offset, 0);
				skb_shinfo(skb)->nr_frags = ++i;
				get_page(pfrag->page);
			}
			copy = min_t(int, copy, pfrag->size - pfrag->offset);
			if (getfrag(from,
				    page_address(pfrag->page) + pfrag->offset,
				    offset, copy, skb->len, skb) < 0)
				goto error_efault;

			pfrag->offset += copy;
			skb_frag_size_add(&skb_shinfo(skb)->frags[i - 1], copy);
			skb->len += copy;
			skb->data_len += copy;
			skb->truesize += copy;
			atomic_add(copy, &sk->sk_wmem_alloc);
		}
		offset += copy;
		length -= copy;
	}

	return 0;

error_efault:
	err = -EFAULT;
error:
	cork->length -= length;
	IP6_INC_STATS(sock_net(sk), rt->rt6i_idev, IPSTATS_MIB_OUTDISCARDS);
	return err;
}
