png_write_image(png_structp png_ptr, png_bytepp image)
{
   png_uint_32 i; /* row index */
   int pass, num_pass; /* pass variables */
   png_bytepp rp; /* points to current row */

   if (png_ptr == NULL)
      return;

   png_debug(1, "in png_write_image");

#ifdef PNG_WRITE_INTERLACING_SUPPORTED
   /* Initialize interlace handling.  If image is not interlaced,
    * this will set pass to 1
    */
   num_pass = png_set_interlace_handling(png_ptr);
#else
   num_pass = 1;
#endif
   /* Loop through passes */
   for (pass = 0; pass < num_pass; pass++)
   {
      /* Loop through image */
      for (i = 0, rp = image; i < png_ptr->height; i++, rp++)
      {
         png_write_row(png_ptr, *rp);
      }
   }
}
