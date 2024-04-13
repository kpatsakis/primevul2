static int nfs4_xdr_enc_remove(struct rpc_rqst *req, __be32 *p, const struct nfs_removeargs *args)
{
	struct xdr_stream xdr;
	struct compound_hdr hdr = {
		.nops = 3,
	};
	int status;

	xdr_init_encode(&xdr, &req->rq_snd_buf, p);
	encode_compound_hdr(&xdr, &hdr);
	if ((status = encode_putfh(&xdr, args->fh)) != 0)
		goto out;
	if ((status = encode_remove(&xdr, &args->name)) != 0)
		goto out;
	status = encode_getfattr(&xdr, args->bitmask);
out:
	return status;
}
