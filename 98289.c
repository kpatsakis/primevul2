static av_cold void avcodec_init(void)
{
    static int initialized = 0;

    if (initialized != 0)
        return;
    initialized = 1;

    if (CONFIG_ME_CMP)
        ff_me_cmp_init_static();
}
