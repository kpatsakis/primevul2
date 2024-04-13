static int sco_sock_connect(struct socket *sock, struct sockaddr *addr, int alen, int flags)
{
	struct sockaddr_sco *sa = (struct sockaddr_sco *) addr;
	struct sock *sk = sock->sk;
	struct hci_dev  *hdev;
	int err;

	BT_DBG("sk %p", sk);

	if (alen < sizeof(struct sockaddr_sco) ||
	    addr->sa_family != AF_BLUETOOTH)
		return -EINVAL;

	if (sk->sk_state != BT_OPEN && sk->sk_state != BT_BOUND)
		return -EBADFD;

	if (sk->sk_type != SOCK_SEQPACKET)
		return -EINVAL;

	hdev = hci_get_route(&sa->sco_bdaddr, &sco_pi(sk)->src, BDADDR_BREDR);
	if (!hdev)
		return -EHOSTUNREACH;
	hci_dev_lock(hdev);

	lock_sock(sk);

	/* Set destination address and psm */
	bacpy(&sco_pi(sk)->dst, &sa->sco_bdaddr);

	err = sco_connect(hdev, sk);
	hci_dev_unlock(hdev);
	hci_dev_put(hdev);
	if (err)
		goto done;

	err = bt_sock_wait_state(sk, BT_CONNECTED,
				 sock_sndtimeo(sk, flags & O_NONBLOCK));

done:
	release_sock(sk);
	return err;
}