png_get_mmx_flagmask (int flag_select, int *compilerID)
{
    /* Obsolete, to be removed from libpng-1.4.0 */
    PNG_UNUSED(flag_select)
    *compilerID = -1;   /* unknown (i.e., no asm/MMX code compiled) */
    return 0L;
}
