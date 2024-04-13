av_cold void ff_idctdsp_init(IDCTDSPContext *c, AVCodecContext *avctx)
{
    const unsigned high_bit_depth = avctx->bits_per_raw_sample > 8;

    if (avctx->lowres==1) {
        c->idct_put  = ff_jref_idct4_put;
        c->idct_add  = ff_jref_idct4_add;
        c->idct      = ff_j_rev_dct4;
        c->perm_type = FF_IDCT_PERM_NONE;
    } else if (avctx->lowres==2) {
        c->idct_put  = ff_jref_idct2_put;
        c->idct_add  = ff_jref_idct2_add;
        c->idct      = ff_j_rev_dct2;
        c->perm_type = FF_IDCT_PERM_NONE;
    } else if (avctx->lowres==3) {
        c->idct_put  = ff_jref_idct1_put;
        c->idct_add  = ff_jref_idct1_add;
        c->idct      = ff_j_rev_dct1;
        c->perm_type = FF_IDCT_PERM_NONE;
    } else {
         if (avctx->bits_per_raw_sample == 10 || avctx->bits_per_raw_sample == 9) {
             /* 10-bit MPEG-4 Simple Studio Profile requires a higher precision IDCT
                However, it only uses idct_put */
            if (c->mpeg4_studio_profile)
                 c->idct_put              = ff_simple_idct_put_int32_10bit;
             else {
                 c->idct_put              = ff_simple_idct_put_int16_10bit;
                c->idct_add              = ff_simple_idct_add_int16_10bit;
                c->idct                  = ff_simple_idct_int16_10bit;
            }
            c->perm_type             = FF_IDCT_PERM_NONE;
        } else if (avctx->bits_per_raw_sample == 12) {
            c->idct_put              = ff_simple_idct_put_int16_12bit;
            c->idct_add              = ff_simple_idct_add_int16_12bit;
            c->idct                  = ff_simple_idct_int16_12bit;
            c->perm_type             = FF_IDCT_PERM_NONE;
        } else {
            if (avctx->idct_algo == FF_IDCT_INT) {
                c->idct_put  = ff_jref_idct_put;
                c->idct_add  = ff_jref_idct_add;
                c->idct      = ff_j_rev_dct;
                c->perm_type = FF_IDCT_PERM_LIBMPEG2;
#if CONFIG_FAANIDCT
            } else if (avctx->idct_algo == FF_IDCT_FAAN) {
                c->idct_put  = ff_faanidct_put;
                c->idct_add  = ff_faanidct_add;
                c->idct      = ff_faanidct;
                c->perm_type = FF_IDCT_PERM_NONE;
#endif /* CONFIG_FAANIDCT */
            } else { // accurate/default
                /* Be sure FF_IDCT_NONE will select this one, since it uses FF_IDCT_PERM_NONE */
                c->idct_put  = ff_simple_idct_put_int16_8bit;
                c->idct_add  = ff_simple_idct_add_int16_8bit;
                c->idct      = ff_simple_idct_int16_8bit;
                c->perm_type = FF_IDCT_PERM_NONE;
            }
        }
    }

    c->put_pixels_clamped        = ff_put_pixels_clamped_c;
    c->put_signed_pixels_clamped = put_signed_pixels_clamped_c;
    c->add_pixels_clamped        = ff_add_pixels_clamped_c;

    if (CONFIG_MPEG4_DECODER && avctx->idct_algo == FF_IDCT_XVID)
        ff_xvid_idct_init(c, avctx);

    if (ARCH_AARCH64)
        ff_idctdsp_init_aarch64(c, avctx, high_bit_depth);
    if (ARCH_ALPHA)
        ff_idctdsp_init_alpha(c, avctx, high_bit_depth);
    if (ARCH_ARM)
        ff_idctdsp_init_arm(c, avctx, high_bit_depth);
    if (ARCH_PPC)
        ff_idctdsp_init_ppc(c, avctx, high_bit_depth);
    if (ARCH_X86)
        ff_idctdsp_init_x86(c, avctx, high_bit_depth);
    if (ARCH_MIPS)
        ff_idctdsp_init_mips(c, avctx, high_bit_depth);

    ff_init_scantable_permutation(c->idct_permutation,
                                  c->perm_type);
}
