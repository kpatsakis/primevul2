static void mcryptd_hash_init(struct crypto_async_request *req_async, int err)
{
	struct mcryptd_hash_ctx *ctx = crypto_tfm_ctx(req_async->tfm);
	struct crypto_shash *child = ctx->child;
	struct ahash_request *req = ahash_request_cast(req_async);
	struct mcryptd_hash_request_ctx *rctx = ahash_request_ctx(req);
	struct shash_desc *desc = &rctx->desc;

	if (unlikely(err == -EINPROGRESS))
		goto out;

	desc->tfm = child;
	desc->flags = CRYPTO_TFM_REQ_MAY_SLEEP;

	err = crypto_shash_init(desc);

	req->base.complete = rctx->complete;

out:
	local_bh_disable();
	rctx->complete(&req->base, err);
	local_bh_enable();
}
