static int llcp_sock_recvmsg(struct kiocb *iocb, struct socket *sock,
			     struct msghdr *msg, size_t len, int flags)
{
	int noblock = flags & MSG_DONTWAIT;
	struct sock *sk = sock->sk;
	unsigned int copied, rlen;
	struct sk_buff *skb, *cskb;
	int err = 0;
 
 	pr_debug("%p %zu\n", sk, len);
 
 	lock_sock(sk);
 
 	if (sk->sk_state == LLCP_CLOSED &&
	    skb_queue_empty(&sk->sk_receive_queue)) {
		release_sock(sk);
		return 0;
	}

	release_sock(sk);

	if (flags & (MSG_OOB))
		return -EOPNOTSUPP;

	skb = skb_recv_datagram(sk, flags, noblock, &err);
	if (!skb) {
		pr_err("Recv datagram failed state %d %d %d",
		       sk->sk_state, err, sock_error(sk));

		if (sk->sk_shutdown & RCV_SHUTDOWN)
			return 0;

		return err;
	}

	rlen = skb->len;		/* real length of skb */
	copied = min_t(unsigned int, rlen, len);

	cskb = skb;
	if (skb_copy_datagram_iovec(cskb, 0, msg->msg_iov, copied)) {
		if (!(flags & MSG_PEEK))
			skb_queue_head(&sk->sk_receive_queue, skb);
		return -EFAULT;
	}

	sock_recv_timestamp(msg, sk, skb);

	if (sk->sk_type == SOCK_DGRAM && msg->msg_name) {
		struct nfc_llcp_ui_cb *ui_cb = nfc_llcp_ui_skb_cb(skb);
		struct sockaddr_nfc_llcp *sockaddr =
			(struct sockaddr_nfc_llcp *) msg->msg_name;

		msg->msg_namelen = sizeof(struct sockaddr_nfc_llcp);

		pr_debug("Datagram socket %d %d\n", ui_cb->dsap, ui_cb->ssap);

		memset(sockaddr, 0, sizeof(*sockaddr));
		sockaddr->sa_family = AF_NFC;
		sockaddr->nfc_protocol = NFC_PROTO_NFC_DEP;
		sockaddr->dsap = ui_cb->dsap;
		sockaddr->ssap = ui_cb->ssap;
	}

	/* Mark read part of skb as used */
	if (!(flags & MSG_PEEK)) {

		/* SOCK_STREAM: re-queue skb if it contains unreceived data */
		if (sk->sk_type == SOCK_STREAM ||
		    sk->sk_type == SOCK_DGRAM ||
		    sk->sk_type == SOCK_RAW) {
			skb_pull(skb, copied);
			if (skb->len) {
				skb_queue_head(&sk->sk_receive_queue, skb);
				goto done;
			}
		}

		kfree_skb(skb);
	}

	/* XXX Queue backlogged skbs */

done:
	/* SOCK_SEQPACKET: return real length if MSG_TRUNC is set */
	if (sk->sk_type == SOCK_SEQPACKET && (flags & MSG_TRUNC))
		copied = rlen;

	return copied;
}
