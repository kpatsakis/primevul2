static int sctp_getsockopt_maxburst(struct sock *sk, int len,
				    char __user *optval,
				    int __user *optlen)
{
	struct sctp_assoc_value params;
	struct sctp_sock *sp;
	struct sctp_association *asoc;

	if (len == sizeof(int)) {
		pr_warn("Use of int in max_burst socket option deprecated\n");
		pr_warn("Use struct sctp_assoc_value instead\n");
		params.assoc_id = 0;
	} else if (len >= sizeof(struct sctp_assoc_value)) {
		len = sizeof(struct sctp_assoc_value);
		if (copy_from_user(&params, optval, len))
			return -EFAULT;
	} else
		return -EINVAL;

	sp = sctp_sk(sk);

	if (params.assoc_id != 0) {
		asoc = sctp_id2assoc(sk, params.assoc_id);
		if (!asoc)
			return -EINVAL;
		params.assoc_value = asoc->max_burst;
	} else
		params.assoc_value = sp->max_burst;

	if (len == sizeof(int)) {
		if (copy_to_user(optval, &params.assoc_value, len))
			return -EFAULT;
	} else {
		if (copy_to_user(optval, &params, len))
			return -EFAULT;
	}

	return 0;

}
