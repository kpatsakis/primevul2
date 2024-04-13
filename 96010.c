static void l2tp_ip6_close(struct sock *sk, long timeout)
{
	write_lock_bh(&l2tp_ip6_lock);
	hlist_del_init(&sk->sk_bind_node);
	sk_del_node_init(sk);
	write_unlock_bh(&l2tp_ip6_lock);

	sk_common_release(sk);
}