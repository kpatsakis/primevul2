int sk_get_filter(struct sock *sk, struct sock_filter __user *ubuf,
		  unsigned int len)
{
	struct sock_fprog_kern *fprog;
	struct sk_filter *filter;
	int ret = 0;

	lock_sock(sk);
	filter = rcu_dereference_protected(sk->sk_filter,
					   sock_owned_by_user(sk));
	if (!filter)
		goto out;

	/* We're copying the filter that has been originally attached,
	 * so no conversion/decode needed anymore.
	 */
	fprog = filter->orig_prog;

	ret = fprog->len;
	if (!len)
		/* User space only enquires number of filter blocks. */
		goto out;

	ret = -EINVAL;
	if (len < fprog->len)
		goto out;

	ret = -EFAULT;
	if (copy_to_user(ubuf, fprog->filter, sk_filter_proglen(fprog)))
		goto out;

	/* Instead of bytes, the API requests to return the number
	 * of filter blocks.
	 */
	ret = fprog->len;
out:
	release_sock(sk);
	return ret;
}