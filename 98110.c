COMPAT_SYSCALL_DEFINE5(mq_timedreceive, mqd_t, mqdes,
		       char __user *, u_msg_ptr,
		       compat_size_t, msg_len, unsigned int __user *, u_msg_prio,
		       const struct compat_timespec __user *, u_abs_timeout)
{
	struct timespec ts, *p = NULL;
	if (u_abs_timeout) {
		int res = compat_prepare_timeout(u_abs_timeout, &ts);
		if (res)
			return res;
		p = &ts;
	}
	return do_mq_timedreceive(mqdes, u_msg_ptr, msg_len, u_msg_prio, p);
}
