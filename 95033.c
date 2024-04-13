smp_fetch_url_ip(const struct arg *args, struct sample *smp, const char *kw, void *private)
{
	struct http_txn *txn;
	struct sockaddr_storage addr;

	CHECK_HTTP_MESSAGE_FIRST();

	txn = smp->strm->txn;
	url2sa(txn->req.chn->buf->p + txn->req.sl.rq.u, txn->req.sl.rq.u_l, &addr, NULL);
	if (((struct sockaddr_in *)&addr)->sin_family != AF_INET)
		return 0;

	smp->data.type = SMP_T_IPV4;
	smp->data.u.ipv4 = ((struct sockaddr_in *)&addr)->sin_addr;
	smp->flags = 0;
	return 1;
}
