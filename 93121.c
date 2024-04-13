 int crypto_rng_reset(struct crypto_rng *tfm, const u8 *seed, unsigned int slen)
 {
 	u8 *buf = NULL;
	int err;

	if (!seed && slen) {
		buf = kmalloc(slen, GFP_KERNEL);
		if (!buf)
			return -ENOMEM;

		get_random_bytes(buf, slen);
 		seed = buf;
 	}
 
	err = crypto_rng_alg(tfm)->seed(tfm, seed, slen);
 
 	kfree(buf);
 	return err;
}
