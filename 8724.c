static int sco_debugfs_show(struct seq_file *f, void *p)
{
	struct sock *sk;

	read_lock(&sco_sk_list.lock);

	sk_for_each(sk, &sco_sk_list.head) {
		seq_printf(f, "%pMR %pMR %d\n", &sco_pi(sk)->src,
			   &sco_pi(sk)->dst, sk->sk_state);
	}

	read_unlock(&sco_sk_list.lock);

	return 0;
}