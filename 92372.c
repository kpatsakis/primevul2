 int DH_check_pub_key(const DH *dh, const BIGNUM *pub_key, int *ret)
 {
     int ok = 0;
    BIGNUM *tmp = NULL;
    BN_CTX *ctx = NULL;
 
     *ret = 0;
    ctx = BN_CTX_new();
    if (ctx == NULL)
         goto err;
    BN_CTX_start(ctx);
    tmp = BN_CTX_get(ctx);
    if (tmp == NULL)
        goto err;
    BN_set_word(tmp, 1);
    if (BN_cmp(pub_key, tmp) <= 0)
         *ret |= DH_CHECK_PUBKEY_TOO_SMALL;
    BN_copy(tmp, dh->p);
    BN_sub_word(tmp, 1);
    if (BN_cmp(pub_key, tmp) >= 0)
         *ret |= DH_CHECK_PUBKEY_TOO_LARGE;
 
    if (dh->q != NULL) {
        /* Check pub_key^q == 1 mod p */
        if (!BN_mod_exp(tmp, pub_key, dh->q, dh->p, ctx))
            goto err;
        if (!BN_is_one(tmp))
            *ret |= DH_CHECK_PUBKEY_INVALID;
    }

     ok = 1;
  err:
    if (ctx != NULL) {
        BN_CTX_end(ctx);
        BN_CTX_free(ctx);
    }
     return (ok);
 }
