int uECC_sign_deterministic(const uint8_t *private_key,
                            const uint8_t *message_hash,
                            unsigned hash_size,
                            const uECC_HashContext *hash_context,
                            uint8_t *signature,
                            uECC_Curve curve) {
    uint8_t *K = hash_context->tmp;
    uint8_t *V = K + hash_context->result_size;
    wordcount_t num_bytes = curve->num_bytes;
    wordcount_t num_n_words = BITS_TO_WORDS(curve->num_n_bits);
    bitcount_t num_n_bits = curve->num_n_bits;
    uECC_word_t tries;
    unsigned i;
    for (i = 0; i < hash_context->result_size; ++i) {
        V[i] = 0x01;
        K[i] = 0;
    }

    /* K = HMAC_K(V || 0x00 || int2octets(x) || h(m)) */
    HMAC_init(hash_context, K);
    V[hash_context->result_size] = 0x00;
    HMAC_update(hash_context, V, hash_context->result_size + 1);
    HMAC_update(hash_context, private_key, num_bytes);
    HMAC_update(hash_context, message_hash, hash_size);
    HMAC_finish(hash_context, K, K);

    update_V(hash_context, K, V);

    /* K = HMAC_K(V || 0x01 || int2octets(x) || h(m)) */
    HMAC_init(hash_context, K);
    V[hash_context->result_size] = 0x01;
    HMAC_update(hash_context, V, hash_context->result_size + 1);
    HMAC_update(hash_context, private_key, num_bytes);
    HMAC_update(hash_context, message_hash, hash_size);
    HMAC_finish(hash_context, K, K);

    update_V(hash_context, K, V);

    for (tries = 0; tries < uECC_RNG_MAX_TRIES; ++tries) {
        uECC_word_t T[uECC_MAX_WORDS];
        uint8_t *T_ptr = (uint8_t *)T;
        wordcount_t T_bytes = 0;
        for (;;) {
            update_V(hash_context, K, V);
            for (i = 0; i < hash_context->result_size; ++i) {
                T_ptr[T_bytes++] = V[i];
                if (T_bytes >= num_n_words * uECC_WORD_SIZE) {
                    goto filled;
                }
            }
        }
    filled:
        if ((bitcount_t)num_n_words * uECC_WORD_SIZE * 8 > num_n_bits) {
            uECC_word_t mask = (uECC_word_t)-1;
            T[num_n_words - 1] &=
                mask >> ((bitcount_t)(num_n_words * uECC_WORD_SIZE * 8 - num_n_bits));
        }

        if (uECC_sign_with_k(private_key, message_hash, hash_size, T, signature, curve)) {
            return 1;
        }

        /* K = HMAC_K(V || 0x00) */
        HMAC_init(hash_context, K);
        V[hash_context->result_size] = 0x00;
        HMAC_update(hash_context, V, hash_context->result_size + 1);
        HMAC_finish(hash_context, K, K);

        update_V(hash_context, K, V);
    }
    return 0;
}