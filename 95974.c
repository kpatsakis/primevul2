int ftrace_set_notrace(struct ftrace_ops *ops, unsigned char *buf,
			int len, int reset)
{
	return ftrace_set_regex(ops, buf, len, reset, 0);
}
