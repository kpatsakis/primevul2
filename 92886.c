int sock_setsockopt(struct socket *sock, int level, int optname,
		    char __user *optval, unsigned int optlen)
{
	struct sock *sk = sock->sk;
	int val;
	int valbool;
	struct linger ling;
	int ret = 0;

	/*
	 *	Options without arguments
	 */

	if (optname == SO_BINDTODEVICE)
		return sock_setbindtodevice(sk, optval, optlen);

	if (optlen < sizeof(int))
		return -EINVAL;

	if (get_user(val, (int __user *)optval))
		return -EFAULT;

	valbool = val ? 1 : 0;

	lock_sock(sk);

	switch (optname) {
	case SO_DEBUG:
		if (val && !capable(CAP_NET_ADMIN))
			ret = -EACCES;
		else
			sock_valbool_flag(sk, SOCK_DBG, valbool);
		break;
	case SO_REUSEADDR:
		sk->sk_reuse = (valbool ? SK_CAN_REUSE : SK_NO_REUSE);
		break;
	case SO_REUSEPORT:
		sk->sk_reuseport = valbool;
		break;
	case SO_TYPE:
	case SO_PROTOCOL:
	case SO_DOMAIN:
	case SO_ERROR:
		ret = -ENOPROTOOPT;
		break;
	case SO_DONTROUTE:
		sock_valbool_flag(sk, SOCK_LOCALROUTE, valbool);
		break;
	case SO_BROADCAST:
		sock_valbool_flag(sk, SOCK_BROADCAST, valbool);
		break;
	case SO_SNDBUF:
		/* Don't error on this BSD doesn't and if you think
		 * about it this is right. Otherwise apps have to
		 * play 'guess the biggest size' games. RCVBUF/SNDBUF
		 * are treated in BSD as hints
		 */
 		val = min_t(u32, val, sysctl_wmem_max);
 set_sndbuf:
 		sk->sk_userlocks |= SOCK_SNDBUF_LOCK;
		sk->sk_sndbuf = max_t(int, val * 2, SOCK_MIN_SNDBUF);
 		/* Wake up sending tasks if we upped the value. */
 		sk->sk_write_space(sk);
 		break;

	case SO_SNDBUFFORCE:
		if (!capable(CAP_NET_ADMIN)) {
			ret = -EPERM;
			break;
		}
		goto set_sndbuf;

	case SO_RCVBUF:
		/* Don't error on this BSD doesn't and if you think
		 * about it this is right. Otherwise apps have to
		 * play 'guess the biggest size' games. RCVBUF/SNDBUF
		 * are treated in BSD as hints
		 */
		val = min_t(u32, val, sysctl_rmem_max);
set_rcvbuf:
		sk->sk_userlocks |= SOCK_RCVBUF_LOCK;
		/*
		 * We double it on the way in to account for
		 * "struct sk_buff" etc. overhead.   Applications
		 * assume that the SO_RCVBUF setting they make will
		 * allow that much actual data to be received on that
		 * socket.
		 *
		 * Applications are unaware that "struct sk_buff" and
		 * other overheads allocate from the receive buffer
		 * during socket buffer allocation.
		 *
		 * And after considering the possible alternatives,
 		 * returning the value we actually used in getsockopt
 		 * is the most desirable behavior.
 		 */
		sk->sk_rcvbuf = max_t(int, val * 2, SOCK_MIN_RCVBUF);
 		break;
 
 	case SO_RCVBUFFORCE:
		if (!capable(CAP_NET_ADMIN)) {
			ret = -EPERM;
			break;
		}
		goto set_rcvbuf;

	case SO_KEEPALIVE:
#ifdef CONFIG_INET
		if (sk->sk_protocol == IPPROTO_TCP &&
		    sk->sk_type == SOCK_STREAM)
			tcp_set_keepalive(sk, valbool);
#endif
		sock_valbool_flag(sk, SOCK_KEEPOPEN, valbool);
		break;

	case SO_OOBINLINE:
		sock_valbool_flag(sk, SOCK_URGINLINE, valbool);
		break;

	case SO_NO_CHECK:
		sk->sk_no_check_tx = valbool;
		break;

	case SO_PRIORITY:
		if ((val >= 0 && val <= 6) ||
		    ns_capable(sock_net(sk)->user_ns, CAP_NET_ADMIN))
			sk->sk_priority = val;
		else
			ret = -EPERM;
		break;

	case SO_LINGER:
		if (optlen < sizeof(ling)) {
			ret = -EINVAL;	/* 1003.1g */
			break;
		}
		if (copy_from_user(&ling, optval, sizeof(ling))) {
			ret = -EFAULT;
			break;
		}
		if (!ling.l_onoff)
			sock_reset_flag(sk, SOCK_LINGER);
		else {
#if (BITS_PER_LONG == 32)
			if ((unsigned int)ling.l_linger >= MAX_SCHEDULE_TIMEOUT/HZ)
				sk->sk_lingertime = MAX_SCHEDULE_TIMEOUT;
			else
#endif
				sk->sk_lingertime = (unsigned int)ling.l_linger * HZ;
			sock_set_flag(sk, SOCK_LINGER);
		}
		break;

	case SO_BSDCOMPAT:
		sock_warn_obsolete_bsdism("setsockopt");
		break;

	case SO_PASSCRED:
		if (valbool)
			set_bit(SOCK_PASSCRED, &sock->flags);
		else
			clear_bit(SOCK_PASSCRED, &sock->flags);
		break;

	case SO_TIMESTAMP:
	case SO_TIMESTAMPNS:
		if (valbool)  {
			if (optname == SO_TIMESTAMP)
				sock_reset_flag(sk, SOCK_RCVTSTAMPNS);
			else
				sock_set_flag(sk, SOCK_RCVTSTAMPNS);
			sock_set_flag(sk, SOCK_RCVTSTAMP);
			sock_enable_timestamp(sk, SOCK_TIMESTAMP);
		} else {
			sock_reset_flag(sk, SOCK_RCVTSTAMP);
			sock_reset_flag(sk, SOCK_RCVTSTAMPNS);
		}
		break;

	case SO_TIMESTAMPING:
		if (val & ~SOF_TIMESTAMPING_MASK) {
			ret = -EINVAL;
			break;
		}

		if (val & SOF_TIMESTAMPING_OPT_ID &&
		    !(sk->sk_tsflags & SOF_TIMESTAMPING_OPT_ID)) {
			if (sk->sk_protocol == IPPROTO_TCP &&
			    sk->sk_type == SOCK_STREAM) {
				if ((1 << sk->sk_state) &
				    (TCPF_CLOSE | TCPF_LISTEN)) {
					ret = -EINVAL;
					break;
				}
				sk->sk_tskey = tcp_sk(sk)->snd_una;
			} else {
				sk->sk_tskey = 0;
			}
		}
		sk->sk_tsflags = val;
		if (val & SOF_TIMESTAMPING_RX_SOFTWARE)
			sock_enable_timestamp(sk,
					      SOCK_TIMESTAMPING_RX_SOFTWARE);
		else
			sock_disable_timestamp(sk,
					       (1UL << SOCK_TIMESTAMPING_RX_SOFTWARE));
		break;

	case SO_RCVLOWAT:
		if (val < 0)
			val = INT_MAX;
		sk->sk_rcvlowat = val ? : 1;
		break;

	case SO_RCVTIMEO:
		ret = sock_set_timeout(&sk->sk_rcvtimeo, optval, optlen);
		break;

	case SO_SNDTIMEO:
		ret = sock_set_timeout(&sk->sk_sndtimeo, optval, optlen);
		break;

	case SO_ATTACH_FILTER:
		ret = -EINVAL;
		if (optlen == sizeof(struct sock_fprog)) {
			struct sock_fprog fprog;

			ret = -EFAULT;
			if (copy_from_user(&fprog, optval, sizeof(fprog)))
				break;

			ret = sk_attach_filter(&fprog, sk);
		}
		break;

	case SO_ATTACH_BPF:
		ret = -EINVAL;
		if (optlen == sizeof(u32)) {
			u32 ufd;

			ret = -EFAULT;
			if (copy_from_user(&ufd, optval, sizeof(ufd)))
				break;

			ret = sk_attach_bpf(ufd, sk);
		}
		break;

	case SO_ATTACH_REUSEPORT_CBPF:
		ret = -EINVAL;
		if (optlen == sizeof(struct sock_fprog)) {
			struct sock_fprog fprog;

			ret = -EFAULT;
			if (copy_from_user(&fprog, optval, sizeof(fprog)))
				break;

			ret = sk_reuseport_attach_filter(&fprog, sk);
		}
		break;

	case SO_ATTACH_REUSEPORT_EBPF:
		ret = -EINVAL;
		if (optlen == sizeof(u32)) {
			u32 ufd;

			ret = -EFAULT;
			if (copy_from_user(&ufd, optval, sizeof(ufd)))
				break;

			ret = sk_reuseport_attach_bpf(ufd, sk);
		}
		break;

	case SO_DETACH_FILTER:
		ret = sk_detach_filter(sk);
		break;

	case SO_LOCK_FILTER:
		if (sock_flag(sk, SOCK_FILTER_LOCKED) && !valbool)
			ret = -EPERM;
		else
			sock_valbool_flag(sk, SOCK_FILTER_LOCKED, valbool);
		break;

	case SO_PASSSEC:
		if (valbool)
			set_bit(SOCK_PASSSEC, &sock->flags);
		else
			clear_bit(SOCK_PASSSEC, &sock->flags);
		break;
	case SO_MARK:
		if (!ns_capable(sock_net(sk)->user_ns, CAP_NET_ADMIN))
			ret = -EPERM;
		else
			sk->sk_mark = val;
		break;

	case SO_RXQ_OVFL:
		sock_valbool_flag(sk, SOCK_RXQ_OVFL, valbool);
		break;

	case SO_WIFI_STATUS:
		sock_valbool_flag(sk, SOCK_WIFI_STATUS, valbool);
		break;

	case SO_PEEK_OFF:
		if (sock->ops->set_peek_off)
			ret = sock->ops->set_peek_off(sk, val);
		else
			ret = -EOPNOTSUPP;
		break;

	case SO_NOFCS:
		sock_valbool_flag(sk, SOCK_NOFCS, valbool);
		break;

	case SO_SELECT_ERR_QUEUE:
		sock_valbool_flag(sk, SOCK_SELECT_ERR_QUEUE, valbool);
		break;

#ifdef CONFIG_NET_RX_BUSY_POLL
	case SO_BUSY_POLL:
		/* allow unprivileged users to decrease the value */
		if ((val > sk->sk_ll_usec) && !capable(CAP_NET_ADMIN))
			ret = -EPERM;
		else {
			if (val < 0)
				ret = -EINVAL;
			else
				sk->sk_ll_usec = val;
		}
		break;
#endif

	case SO_MAX_PACING_RATE:
		sk->sk_max_pacing_rate = val;
		sk->sk_pacing_rate = min(sk->sk_pacing_rate,
					 sk->sk_max_pacing_rate);
		break;

	case SO_INCOMING_CPU:
		sk->sk_incoming_cpu = val;
		break;

	case SO_CNX_ADVICE:
		if (val == 1)
			dst_negative_advice(sk);
		break;
	default:
		ret = -ENOPROTOOPT;
		break;
	}
	release_sock(sk);
	return ret;
}
