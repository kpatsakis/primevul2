static int sctp_getsockopt_pr_streamstatus(struct sock *sk, int len,
					   char __user *optval,
					   int __user *optlen)
{
	struct sctp_stream_out *streamout;
	struct sctp_association *asoc;
	struct sctp_prstatus params;
	int retval = -EINVAL;
	int policy;

	if (len < sizeof(params))
		goto out;

	len = sizeof(params);
	if (copy_from_user(&params, optval, len)) {
		retval = -EFAULT;
		goto out;
	}

	policy = params.sprstat_policy;
	if (policy & ~SCTP_PR_SCTP_MASK)
		goto out;

	asoc = sctp_id2assoc(sk, params.sprstat_assoc_id);
	if (!asoc || params.sprstat_sid >= asoc->stream.outcnt)
		goto out;

	streamout = &asoc->stream.out[params.sprstat_sid];
	if (policy == SCTP_PR_SCTP_NONE) {
		params.sprstat_abandoned_unsent = 0;
		params.sprstat_abandoned_sent = 0;
		for (policy = 0; policy <= SCTP_PR_INDEX(MAX); policy++) {
			params.sprstat_abandoned_unsent +=
				streamout->abandoned_unsent[policy];
			params.sprstat_abandoned_sent +=
				streamout->abandoned_sent[policy];
		}
	} else {
		params.sprstat_abandoned_unsent =
			streamout->abandoned_unsent[__SCTP_PR_INDEX(policy)];
		params.sprstat_abandoned_sent =
			streamout->abandoned_sent[__SCTP_PR_INDEX(policy)];
	}

	if (put_user(len, optlen) || copy_to_user(optval, &params, len)) {
		retval = -EFAULT;
		goto out;
	}

	retval = 0;

out:
	return retval;
}
