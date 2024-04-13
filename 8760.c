static void sco_conn_ready(struct sco_conn *conn)
{
	struct sock *parent;
	struct sock *sk = conn->sk;

	BT_DBG("conn %p", conn);

	if (sk) {
		lock_sock(sk);
		sco_sock_clear_timer(sk);
		sk->sk_state = BT_CONNECTED;
		sk->sk_state_change(sk);
		release_sock(sk);
	} else {
		sco_conn_lock(conn);

		if (!conn->hcon) {
			sco_conn_unlock(conn);
			return;
		}

		parent = sco_get_sock_listen(&conn->hcon->src);
		if (!parent) {
			sco_conn_unlock(conn);
			return;
		}

		lock_sock(parent);

		sk = sco_sock_alloc(sock_net(parent), NULL,
				    BTPROTO_SCO, GFP_ATOMIC, 0);
		if (!sk) {
			release_sock(parent);
			sco_conn_unlock(conn);
			return;
		}

		sco_sock_init(sk, parent);

		bacpy(&sco_pi(sk)->src, &conn->hcon->src);
		bacpy(&sco_pi(sk)->dst, &conn->hcon->dst);

		hci_conn_hold(conn->hcon);
		__sco_chan_add(conn, sk, parent);

		if (test_bit(BT_SK_DEFER_SETUP, &bt_sk(parent)->flags))
			sk->sk_state = BT_CONNECT2;
		else
			sk->sk_state = BT_CONNECTED;

		/* Wake up parent */
		parent->sk_data_ready(parent);

		release_sock(parent);

		sco_conn_unlock(conn);
	}
}