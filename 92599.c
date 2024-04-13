static int l2tp_ip6_recvmsg(struct kiocb *iocb, struct sock *sk,
			    struct msghdr *msg, size_t len, int noblock,
			    int flags, int *addr_len)
{
	struct ipv6_pinfo *np = inet6_sk(sk);
	struct sockaddr_l2tpip6 *lsa = (struct sockaddr_l2tpip6 *)msg->msg_name;
	size_t copied = 0;
	int err = -EOPNOTSUPP;
	struct sk_buff *skb;

	if (flags & MSG_OOB)
		goto out;

	if (addr_len)
		*addr_len = sizeof(*lsa);

	if (flags & MSG_ERRQUEUE)
		return ipv6_recv_error(sk, msg, len);

	skb = skb_recv_datagram(sk, flags, noblock, &err);
	if (!skb)
		goto out;

	copied = skb->len;
	if (len < copied) {
		msg->msg_flags |= MSG_TRUNC;
		copied = len;
	}

	err = skb_copy_datagram_iovec(skb, 0, msg->msg_iov, copied);
	if (err)
		goto done;

	sock_recv_timestamp(msg, sk, skb);

	/* Copy the address. */
	if (lsa) {
		lsa->l2tp_family = AF_INET6;
		lsa->l2tp_unused = 0;
 		lsa->l2tp_addr = ipv6_hdr(skb)->saddr;
 		lsa->l2tp_flowinfo = 0;
 		lsa->l2tp_scope_id = 0;
		lsa->l2tp_conn_id = 0;
 		if (ipv6_addr_type(&lsa->l2tp_addr) & IPV6_ADDR_LINKLOCAL)
 			lsa->l2tp_scope_id = IP6CB(skb)->iif;
 	}

	if (np->rxopt.all)
		ip6_datagram_recv_ctl(sk, msg, skb);

	if (flags & MSG_TRUNC)
		copied = skb->len;
done:
	skb_free_datagram(sk, skb);
out:
	return err ? err : copied;
}
