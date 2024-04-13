static void sco_sock_cleanup_listen(struct sock *parent)
{
	struct sock *sk;

	BT_DBG("parent %p", parent);

	/* Close not yet accepted channels */
	while ((sk = bt_accept_dequeue(parent, NULL))) {
		sco_sock_close(sk);
		sco_sock_kill(sk);
	}

	parent->sk_state  = BT_CLOSED;
	sock_set_flag(parent, SOCK_ZAPPED);
}
