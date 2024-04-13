static int iucv_sock_recvmsg(struct kiocb *iocb, struct socket *sock,
			     struct msghdr *msg, size_t len, int flags)
{
	int noblock = flags & MSG_DONTWAIT;
	struct sock *sk = sock->sk;
	struct iucv_sock *iucv = iucv_sk(sk);
	unsigned int copied, rlen;
 	struct sk_buff *skb, *rskb, *cskb;
 	int err = 0;
 
	msg->msg_namelen = 0;

 	if ((sk->sk_state == IUCV_DISCONN) &&
 	    skb_queue_empty(&iucv->backlog_skb_q) &&
 	    skb_queue_empty(&sk->sk_receive_queue) &&
	    list_empty(&iucv->message_q.list))
		return 0;

	if (flags & (MSG_OOB))
		return -EOPNOTSUPP;

	/* receive/dequeue next skb:
	 * the function understands MSG_PEEK and, thus, does not dequeue skb */
	skb = skb_recv_datagram(sk, flags, noblock, &err);
	if (!skb) {
		if (sk->sk_shutdown & RCV_SHUTDOWN)
			return 0;
		return err;
	}

	rlen   = skb->len;		/* real length of skb */
	copied = min_t(unsigned int, rlen, len);
	if (!rlen)
		sk->sk_shutdown = sk->sk_shutdown | RCV_SHUTDOWN;

	cskb = skb;
	if (skb_copy_datagram_iovec(cskb, 0, msg->msg_iov, copied)) {
		if (!(flags & MSG_PEEK))
			skb_queue_head(&sk->sk_receive_queue, skb);
		return -EFAULT;
	}

	/* SOCK_SEQPACKET: set MSG_TRUNC if recv buf size is too small */
	if (sk->sk_type == SOCK_SEQPACKET) {
		if (copied < rlen)
			msg->msg_flags |= MSG_TRUNC;
		/* each iucv message contains a complete record */
		msg->msg_flags |= MSG_EOR;
	}

	/* create control message to store iucv msg target class:
	 * get the trgcls from the control buffer of the skb due to
	 * fragmentation of original iucv message. */
	err = put_cmsg(msg, SOL_IUCV, SCM_IUCV_TRGCLS,
			CB_TRGCLS_LEN, CB_TRGCLS(skb));
	if (err) {
		if (!(flags & MSG_PEEK))
			skb_queue_head(&sk->sk_receive_queue, skb);
		return err;
	}

	/* Mark read part of skb as used */
	if (!(flags & MSG_PEEK)) {

		/* SOCK_STREAM: re-queue skb if it contains unreceived data */
		if (sk->sk_type == SOCK_STREAM) {
			skb_pull(skb, copied);
			if (skb->len) {
				skb_queue_head(&sk->sk_receive_queue, skb);
				goto done;
			}
		}

		kfree_skb(skb);
		if (iucv->transport == AF_IUCV_TRANS_HIPER) {
			atomic_inc(&iucv->msg_recv);
			if (atomic_read(&iucv->msg_recv) > iucv->msglimit) {
				WARN_ON(1);
				iucv_sock_close(sk);
				return -EFAULT;
			}
		}

		/* Queue backlog skbs */
		spin_lock_bh(&iucv->message_q.lock);
		rskb = skb_dequeue(&iucv->backlog_skb_q);
		while (rskb) {
			if (sock_queue_rcv_skb(sk, rskb)) {
				skb_queue_head(&iucv->backlog_skb_q,
						rskb);
				break;
			} else {
				rskb = skb_dequeue(&iucv->backlog_skb_q);
			}
		}
		if (skb_queue_empty(&iucv->backlog_skb_q)) {
			if (!list_empty(&iucv->message_q.list))
				iucv_process_message_q(sk);
			if (atomic_read(&iucv->msg_recv) >=
							iucv->msglimit / 2) {
				err = iucv_send_ctrl(sk, AF_IUCV_FLAG_WIN);
				if (err) {
					sk->sk_state = IUCV_DISCONN;
					sk->sk_state_change(sk);
				}
			}
		}
		spin_unlock_bh(&iucv->message_q.lock);
	}

done:
	/* SOCK_SEQPACKET: return real length if MSG_TRUNC is set */
	if (sk->sk_type == SOCK_SEQPACKET && (flags & MSG_TRUNC))
		copied = rlen;

	return copied;
}
