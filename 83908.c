png_write_row(png_structp png_ptr, png_bytep row)
{
   if (png_ptr == NULL)
      return;

   png_debug2(1, "in png_write_row (row %ld, pass %d)",
      png_ptr->row_number, png_ptr->pass);

   /* Initialize transformations and other stuff if first time */
   if (png_ptr->row_number == 0 && png_ptr->pass == 0)
   {
      /* Make sure we wrote the header info */
      if (!(png_ptr->mode & PNG_WROTE_INFO_BEFORE_PLTE))
         png_error(png_ptr,
            "png_write_info was never called before png_write_row.");

      /* Check for transforms that have been set but were defined out */
#if !defined(PNG_WRITE_INVERT_SUPPORTED) && defined(PNG_READ_INVERT_SUPPORTED)
      if (png_ptr->transformations & PNG_INVERT_MONO)
         png_warning(png_ptr,
             "PNG_WRITE_INVERT_SUPPORTED is not defined.");
#endif
#if !defined(PNG_WRITE_FILLER_SUPPORTED) && defined(PNG_READ_FILLER_SUPPORTED)
      if (png_ptr->transformations & PNG_FILLER)
         png_warning(png_ptr,
             "PNG_WRITE_FILLER_SUPPORTED is not defined.");
#endif
#if !defined(PNG_WRITE_PACKSWAP_SUPPORTED) && \
    defined(PNG_READ_PACKSWAP_SUPPORTED)
      if (png_ptr->transformations & PNG_PACKSWAP)
         png_warning(png_ptr,
             "PNG_WRITE_PACKSWAP_SUPPORTED is not defined.");
#endif
#if !defined(PNG_WRITE_PACK_SUPPORTED) && defined(PNG_READ_PACK_SUPPORTED)
      if (png_ptr->transformations & PNG_PACK)
         png_warning(png_ptr, "PNG_WRITE_PACK_SUPPORTED is not defined.");
#endif
#if !defined(PNG_WRITE_SHIFT_SUPPORTED) && defined(PNG_READ_SHIFT_SUPPORTED)
      if (png_ptr->transformations & PNG_SHIFT)
         png_warning(png_ptr, "PNG_WRITE_SHIFT_SUPPORTED is not defined.");
#endif
#if !defined(PNG_WRITE_BGR_SUPPORTED) && defined(PNG_READ_BGR_SUPPORTED)
      if (png_ptr->transformations & PNG_BGR)
         png_warning(png_ptr, "PNG_WRITE_BGR_SUPPORTED is not defined.");
#endif
#if !defined(PNG_WRITE_SWAP_SUPPORTED) && defined(PNG_READ_SWAP_SUPPORTED)
      if (png_ptr->transformations & PNG_SWAP_BYTES)
         png_warning(png_ptr, "PNG_WRITE_SWAP_SUPPORTED is not defined.");
#endif

      png_write_start_row(png_ptr);
   }

#ifdef PNG_WRITE_INTERLACING_SUPPORTED
   /* If interlaced and not interested in row, return */
   if (png_ptr->interlaced && (png_ptr->transformations & PNG_INTERLACE))
   {
      switch (png_ptr->pass)
      {
         case 0:
            if (png_ptr->row_number & 0x07)
            {
               png_write_finish_row(png_ptr);
               return;
            }
            break;
         case 1:
            if ((png_ptr->row_number & 0x07) || png_ptr->width < 5)
            {
               png_write_finish_row(png_ptr);
               return;
            }
            break;
         case 2:
            if ((png_ptr->row_number & 0x07) != 4)
            {
               png_write_finish_row(png_ptr);
               return;
            }
            break;
         case 3:
            if ((png_ptr->row_number & 0x03) || png_ptr->width < 3)
            {
               png_write_finish_row(png_ptr);
               return;
            }
            break;
         case 4:
            if ((png_ptr->row_number & 0x03) != 2)
            {
               png_write_finish_row(png_ptr);
               return;
            }
            break;
         case 5:
            if ((png_ptr->row_number & 0x01) || png_ptr->width < 2)
            {
               png_write_finish_row(png_ptr);
               return;
            }
            break;
         case 6:
            if (!(png_ptr->row_number & 0x01))
            {
               png_write_finish_row(png_ptr);
               return;
            }
            break;
      }
   }
#endif

   /* Set up row info for transformations */
   png_ptr->row_info.color_type = png_ptr->color_type;
   png_ptr->row_info.width = png_ptr->usr_width;
   png_ptr->row_info.channels = png_ptr->usr_channels;
   png_ptr->row_info.bit_depth = png_ptr->usr_bit_depth;
   png_ptr->row_info.pixel_depth = (png_byte)(png_ptr->row_info.bit_depth *
      png_ptr->row_info.channels);

   png_ptr->row_info.rowbytes = PNG_ROWBYTES(png_ptr->row_info.pixel_depth,
      png_ptr->row_info.width);

   png_debug1(3, "row_info->color_type = %d", png_ptr->row_info.color_type);
   png_debug1(3, "row_info->width = %lu", png_ptr->row_info.width);
   png_debug1(3, "row_info->channels = %d", png_ptr->row_info.channels);
   png_debug1(3, "row_info->bit_depth = %d", png_ptr->row_info.bit_depth);
   png_debug1(3, "row_info->pixel_depth = %d", png_ptr->row_info.pixel_depth);
   png_debug1(3, "row_info->rowbytes = %lu", png_ptr->row_info.rowbytes);

   /* Copy user's row into buffer, leaving room for filter byte. */
   png_memcpy_check(png_ptr, png_ptr->row_buf + 1, row,
      png_ptr->row_info.rowbytes);

#ifdef PNG_WRITE_INTERLACING_SUPPORTED
   /* Handle interlacing */
   if (png_ptr->interlaced && png_ptr->pass < 6 &&
      (png_ptr->transformations & PNG_INTERLACE))
   {
      png_do_write_interlace(&(png_ptr->row_info),
         png_ptr->row_buf + 1, png_ptr->pass);
      /* This should always get caught above, but still ... */
      if (!(png_ptr->row_info.width))
      {
         png_write_finish_row(png_ptr);
         return;
      }
   }
#endif

   /* Handle other transformations */
   if (png_ptr->transformations)
      png_do_write_transformations(png_ptr);

#ifdef PNG_MNG_FEATURES_SUPPORTED
   /* Write filter_method 64 (intrapixel differencing) only if
    * 1. Libpng was compiled with PNG_MNG_FEATURES_SUPPORTED and
    * 2. Libpng did not write a PNG signature (this filter_method is only
    *    used in PNG datastreams that are embedded in MNG datastreams) and
    * 3. The application called png_permit_mng_features with a mask that
    *    included PNG_FLAG_MNG_FILTER_64 and
    * 4. The filter_method is 64 and
    * 5. The color_type is RGB or RGBA
    */
   if ((png_ptr->mng_features_permitted & PNG_FLAG_MNG_FILTER_64) &&
      (png_ptr->filter_type == PNG_INTRAPIXEL_DIFFERENCING))
   {
      /* Intrapixel differencing */
      png_do_write_intrapixel(&(png_ptr->row_info), png_ptr->row_buf + 1);
   }
#endif

   /* Find a filter if necessary, filter the row and write it out. */
   png_write_find_filter(png_ptr, &(png_ptr->row_info));

   if (png_ptr->write_row_fn != NULL)
      (*(png_ptr->write_row_fn))(png_ptr, png_ptr->row_number, png_ptr->pass);
}
