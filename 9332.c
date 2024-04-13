teardown_digests(cms_context *ctx)
{
	struct digest *digests = ctx->digests;

	if (!digests)
		return;

	for (int i = 0; i < n_digest_params; i++) {
		if (digests[i].pk11ctx) {
			PK11_Finalize(digests[i].pk11ctx);
			PK11_DestroyContext(digests[i].pk11ctx, PR_TRUE);
		}
		if (digests[i].pe_digest) {
			/* XXX sure seems like we should be freeing it here,
			 * but that's segfaulting, and we know it'll get
			 * cleaned up with PORT_FreeArena a couple of lines
			 * down.
			 */
			digests[i].pe_digest = NULL;
		}
	}
	PORT_Free(digests);
	ctx->digests = NULL;
}