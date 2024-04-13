png_set_mmx_thresholds (png_structp png_ptr,
                        png_byte mmx_bitdepth_threshold,
                        png_uint_32 mmx_rowbytes_threshold)
{
/* Obsolete as of libpng-1.2.20 and will be removed from libpng-1.4.0 */
    if (png_ptr == NULL)
       return;
    /* Quiet the compiler */
    PNG_UNUSED(mmx_bitdepth_threshold)
    PNG_UNUSED(mmx_rowbytes_threshold)
}
