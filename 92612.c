static int crypto_blkcipher_report(struct sk_buff *skb, struct crypto_alg *alg)
 {
 	struct crypto_report_blkcipher rblkcipher;
 
	strncpy(rblkcipher.type, "blkcipher", sizeof(rblkcipher.type));
	strncpy(rblkcipher.geniv, alg->cra_blkcipher.geniv ?: "<default>",
		sizeof(rblkcipher.geniv));
 
 	rblkcipher.blocksize = alg->cra_blocksize;
 	rblkcipher.min_keysize = alg->cra_blkcipher.min_keysize;
	rblkcipher.max_keysize = alg->cra_blkcipher.max_keysize;
	rblkcipher.ivsize = alg->cra_blkcipher.ivsize;

	if (nla_put(skb, CRYPTOCFGA_REPORT_BLKCIPHER,
		    sizeof(struct crypto_report_blkcipher), &rblkcipher))
		goto nla_put_failure;
	return 0;

nla_put_failure:
	return -EMSGSIZE;
}
