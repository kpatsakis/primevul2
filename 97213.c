static int sha256_ssse3_export(struct shash_desc *desc, void *out)
{
	struct sha256_state *sctx = shash_desc_ctx(desc);

	memcpy(out, sctx, sizeof(*sctx));

	return 0;
}