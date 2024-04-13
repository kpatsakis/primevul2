int snmp_helper(void *context, size_t hdrlen, unsigned char tag,
 		const void *data, size_t datalen)
 {
 	struct snmp_ctx *ctx = (struct snmp_ctx *)context;
	__be32 *pdata;
 
	if (datalen != 4)
		return -EINVAL;
	pdata = (__be32 *)data;
 	if (*pdata == ctx->from) {
 		pr_debug("%s: %pI4 to %pI4\n", __func__,
 			 (void *)&ctx->from, (void *)&ctx->to);

		if (*ctx->check)
			fast_csum(ctx, (unsigned char *)data - ctx->begin);
		*pdata = ctx->to;
	}

	return 1;
}