static int sctp_getsockopt_autoclose(struct sock *sk, int len, char __user *optval, int __user *optlen)
{
	/* Applicable to UDP-style socket only */
	if (sctp_style(sk, TCP))
		return -EOPNOTSUPP;
	if (len != sizeof(int))
		return -EINVAL;
	if (copy_to_user(optval, &sctp_sk(sk)->autoclose, len))
		return -EFAULT;
	return 0;
}
