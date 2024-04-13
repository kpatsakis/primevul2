png_write_png(png_structp png_ptr, png_infop info_ptr,
              int transforms, voidp params)
{
   if (png_ptr == NULL || info_ptr == NULL)
      return;

   /* Write the file header information. */
   png_write_info(png_ptr, info_ptr);

   /* ------ these transformations don't touch the info structure ------- */

#ifdef PNG_WRITE_INVERT_SUPPORTED
   /* Invert monochrome pixels */
   if (transforms & PNG_TRANSFORM_INVERT_MONO)
      png_set_invert_mono(png_ptr);
#endif

#ifdef PNG_WRITE_SHIFT_SUPPORTED
   /* Shift the pixels up to a legal bit depth and fill in
    * as appropriate to correctly scale the image.
    */
   if ((transforms & PNG_TRANSFORM_SHIFT)
               && (info_ptr->valid & PNG_INFO_sBIT))
      png_set_shift(png_ptr, &info_ptr->sig_bit);
#endif

#ifdef PNG_WRITE_PACK_SUPPORTED
   /* Pack pixels into bytes */
   if (transforms & PNG_TRANSFORM_PACKING)
       png_set_packing(png_ptr);
#endif

#ifdef PNG_WRITE_SWAP_ALPHA_SUPPORTED
   /* Swap location of alpha bytes from ARGB to RGBA */
   if (transforms & PNG_TRANSFORM_SWAP_ALPHA)
      png_set_swap_alpha(png_ptr);
#endif

#ifdef PNG_WRITE_FILLER_SUPPORTED
   /* Pack XRGB/RGBX/ARGB/RGBA into * RGB (4 channels -> 3 channels) */
   if (transforms & PNG_TRANSFORM_STRIP_FILLER_AFTER)
      png_set_filler(png_ptr, 0, PNG_FILLER_AFTER);
   else if (transforms & PNG_TRANSFORM_STRIP_FILLER_BEFORE)
      png_set_filler(png_ptr, 0, PNG_FILLER_BEFORE);
#endif

#ifdef PNG_WRITE_BGR_SUPPORTED
   /* Flip BGR pixels to RGB */
   if (transforms & PNG_TRANSFORM_BGR)
      png_set_bgr(png_ptr);
#endif

#ifdef PNG_WRITE_SWAP_SUPPORTED
   /* Swap bytes of 16-bit files to most significant byte first */
   if (transforms & PNG_TRANSFORM_SWAP_ENDIAN)
      png_set_swap(png_ptr);
#endif

#ifdef PNG_WRITE_PACKSWAP_SUPPORTED
   /* Swap bits of 1, 2, 4 bit packed pixel formats */
   if (transforms & PNG_TRANSFORM_PACKSWAP)
      png_set_packswap(png_ptr);
#endif

#ifdef PNG_WRITE_INVERT_ALPHA_SUPPORTED
   /* Invert the alpha channel from opacity to transparency */
   if (transforms & PNG_TRANSFORM_INVERT_ALPHA)
      png_set_invert_alpha(png_ptr);
#endif

   /* ----------------------- end of transformations ------------------- */

   /* Write the bits */
   if (info_ptr->valid & PNG_INFO_IDAT)
       png_write_image(png_ptr, info_ptr->row_pointers);

   /* It is REQUIRED to call this to finish writing the rest of the file */
   png_write_end(png_ptr, info_ptr);

   PNG_UNUSED(transforms) /* Quiet compiler warnings */
   PNG_UNUSED(params)
}
