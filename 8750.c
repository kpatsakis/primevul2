static int sco_sock_setsockopt(struct socket *sock, int level, int optname,
			       sockptr_t optval, unsigned int optlen)
{
	struct sock *sk = sock->sk;
	int len, err = 0;
	struct bt_voice voice;
	u32 opt;

	BT_DBG("sk %p", sk);

	lock_sock(sk);

	switch (optname) {

	case BT_DEFER_SETUP:
		if (sk->sk_state != BT_BOUND && sk->sk_state != BT_LISTEN) {
			err = -EINVAL;
			break;
		}

		if (copy_from_sockptr(&opt, optval, sizeof(u32))) {
			err = -EFAULT;
			break;
		}

		if (opt)
			set_bit(BT_SK_DEFER_SETUP, &bt_sk(sk)->flags);
		else
			clear_bit(BT_SK_DEFER_SETUP, &bt_sk(sk)->flags);
		break;

	case BT_VOICE:
		if (sk->sk_state != BT_OPEN && sk->sk_state != BT_BOUND &&
		    sk->sk_state != BT_CONNECT2) {
			err = -EINVAL;
			break;
		}

		voice.setting = sco_pi(sk)->setting;

		len = min_t(unsigned int, sizeof(voice), optlen);
		if (copy_from_sockptr(&voice, optval, len)) {
			err = -EFAULT;
			break;
		}

		/* Explicitly check for these values */
		if (voice.setting != BT_VOICE_TRANSPARENT &&
		    voice.setting != BT_VOICE_CVSD_16BIT) {
			err = -EINVAL;
			break;
		}

		sco_pi(sk)->setting = voice.setting;
		break;

	case BT_PKT_STATUS:
		if (copy_from_sockptr(&opt, optval, sizeof(u32))) {
			err = -EFAULT;
			break;
		}

		if (opt)
			sco_pi(sk)->cmsg_mask |= SCO_CMSG_PKT_STATUS;
		else
			sco_pi(sk)->cmsg_mask &= SCO_CMSG_PKT_STATUS;
		break;

	default:
		err = -ENOPROTOOPT;
		break;
	}

	release_sock(sk);
	return err;
}