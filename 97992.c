static void update_options(char **dest, const char *name, void *src)
{
    av_freep(dest);
    av_opt_get(src, name, AV_OPT_SEARCH_CHILDREN, (uint8_t**)dest);
    if (*dest && !strlen(*dest))
        av_freep(dest);
}
