static int netlink_dump(struct sock *sk)
{
	struct netlink_sock *nlk = nlk_sk(sk);
 	struct netlink_callback *cb;
 	struct sk_buff *skb = NULL;
 	struct nlmsghdr *nlh;
	struct module *module;
 	int len, err = -ENOBUFS;
 	int alloc_min_size;
 	int alloc_size;

	mutex_lock(nlk->cb_mutex);
	if (!nlk->cb_running) {
		err = -EINVAL;
		goto errout_skb;
	}

	if (atomic_read(&sk->sk_rmem_alloc) >= sk->sk_rcvbuf)
		goto errout_skb;

	/* NLMSG_GOODSIZE is small to avoid high order allocations being
	 * required, but it makes sense to _attempt_ a 16K bytes allocation
	 * to reduce number of system calls on dump operations, if user
	 * ever provided a big enough buffer.
	 */
	cb = &nlk->cb;
	alloc_min_size = max_t(int, cb->min_dump_alloc, NLMSG_GOODSIZE);

	if (alloc_min_size < nlk->max_recvmsg_len) {
		alloc_size = nlk->max_recvmsg_len;
		skb = alloc_skb(alloc_size, GFP_KERNEL |
					    __GFP_NOWARN | __GFP_NORETRY);
	}
	if (!skb) {
		alloc_size = alloc_min_size;
		skb = alloc_skb(alloc_size, GFP_KERNEL);
	}
	if (!skb)
		goto errout_skb;

	/* Trim skb to allocated size. User is expected to provide buffer as
	 * large as max(min_dump_alloc, 16KiB (mac_recvmsg_len capped at
	 * netlink_recvmsg())). dump will pack as many smaller messages as
	 * could fit within the allocated skb. skb is typically allocated
	 * with larger space than required (could be as much as near 2x the
	 * requested size with align to next power of 2 approach). Allowing
	 * dump to use the excess space makes it difficult for a user to have a
	 * reasonable static buffer based on the expected largest dump of a
	 * single netdev. The outcome is MSG_TRUNC error.
	 */
	skb_reserve(skb, skb_tailroom(skb) - alloc_size);
	netlink_skb_set_owner_r(skb, sk);

	len = cb->dump(skb, cb);

	if (len > 0) {
		mutex_unlock(nlk->cb_mutex);

		if (sk_filter(sk, skb))
			kfree_skb(skb);
		else
			__netlink_sendskb(sk, skb);
		return 0;
	}

	nlh = nlmsg_put_answer(skb, cb, NLMSG_DONE, sizeof(len), NLM_F_MULTI);
	if (!nlh)
		goto errout_skb;

	nl_dump_check_consistent(cb, nlh);

	memcpy(nlmsg_data(nlh), &len, sizeof(len));

	if (sk_filter(sk, skb))
		kfree_skb(skb);
	else
		__netlink_sendskb(sk, skb);

	if (cb->done)
 		cb->done(cb);
 
 	nlk->cb_running = false;
	module = cb->module;
	skb = cb->skb;
 	mutex_unlock(nlk->cb_mutex);
	module_put(module);
	consume_skb(skb);
 	return 0;
 
 errout_skb:
	mutex_unlock(nlk->cb_mutex);
	kfree_skb(skb);
	return err;
}