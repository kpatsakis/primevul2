png_do_read_transformations(png_structp png_ptr)
{
   png_debug(1, "in png_do_read_transformations");

   if (png_ptr->row_buf == NULL)
   {
#if defined(PNG_STDIO_SUPPORTED) && !defined(_WIN32_WCE)
      char msg[50];

      png_snprintf2(msg, 50,
         "NULL row buffer for row %ld, pass %d", (long)png_ptr->row_number,
         png_ptr->pass);
      png_error(png_ptr, msg);
#else
      png_error(png_ptr, "NULL row buffer");
#endif
   }
#ifdef PNG_WARN_UNINITIALIZED_ROW
   if (!(png_ptr->flags & PNG_FLAG_ROW_INIT))
      /* Application has failed to call either png_read_start_image()
       * or png_read_update_info() after setting transforms that expand
       * pixels.  This check added to libpng-1.2.19
       */
#if (PNG_WARN_UNINITIALIZED_ROW==1)
      png_error(png_ptr, "Uninitialized row");
#else
      png_warning(png_ptr, "Uninitialized row");
#endif
#endif

#ifdef PNG_READ_EXPAND_SUPPORTED
   if (png_ptr->transformations & PNG_EXPAND)
   {
      if (png_ptr->row_info.color_type == PNG_COLOR_TYPE_PALETTE)
      {
         if (png_ptr->palette == NULL)
            png_error (png_ptr, "Palette is NULL in indexed image");

         png_do_expand_palette(&(png_ptr->row_info), png_ptr->row_buf + 1,
            png_ptr->palette, png_ptr->trans, png_ptr->num_trans);
      }
      else
      {
         if (png_ptr->num_trans &&
             (png_ptr->transformations & PNG_EXPAND_tRNS))
            png_do_expand(&(png_ptr->row_info), png_ptr->row_buf + 1,
               &(png_ptr->trans_values));
         else
            png_do_expand(&(png_ptr->row_info), png_ptr->row_buf + 1,
               NULL);
      }
   }
#endif

#ifdef PNG_READ_STRIP_ALPHA_SUPPORTED
   if (png_ptr->flags & PNG_FLAG_STRIP_ALPHA)
      png_do_strip_filler(&(png_ptr->row_info), png_ptr->row_buf + 1,
         PNG_FLAG_FILLER_AFTER | (png_ptr->flags & PNG_FLAG_STRIP_ALPHA));
#endif

#ifdef PNG_READ_RGB_TO_GRAY_SUPPORTED
   if (png_ptr->transformations & PNG_RGB_TO_GRAY)
   {
      int rgb_error =
         png_do_rgb_to_gray(png_ptr, &(png_ptr->row_info),
             png_ptr->row_buf + 1);
      if (rgb_error)
      {
         png_ptr->rgb_to_gray_status=1;
         if ((png_ptr->transformations & PNG_RGB_TO_GRAY) ==
             PNG_RGB_TO_GRAY_WARN)
            png_warning(png_ptr, "png_do_rgb_to_gray found nongray pixel");
         if ((png_ptr->transformations & PNG_RGB_TO_GRAY) ==
             PNG_RGB_TO_GRAY_ERR)
            png_error(png_ptr, "png_do_rgb_to_gray found nongray pixel");
      }
   }
#endif

/* From Andreas Dilger e-mail to png-implement, 26 March 1998:
 *
 *   In most cases, the "simple transparency" should be done prior to doing
 *   gray-to-RGB, or you will have to test 3x as many bytes to check if a
 *   pixel is transparent.  You would also need to make sure that the
 *   transparency information is upgraded to RGB.
 *
 *   To summarize, the current flow is:
 *   - Gray + simple transparency -> compare 1 or 2 gray bytes and composite
 *                                   with background "in place" if transparent,
 *                                   convert to RGB if necessary
 *   - Gray + alpha -> composite with gray background and remove alpha bytes,
 *                                   convert to RGB if necessary
 *
 *   To support RGB backgrounds for gray images we need:
 *   - Gray + simple transparency -> convert to RGB + simple transparency,
 *                                   compare 3 or 6 bytes and composite with
 *                                   background "in place" if transparent
 *                                   (3x compare/pixel compared to doing
 *                                   composite with gray bkgrnd)
 *   - Gray + alpha -> convert to RGB + alpha, composite with background and
 *                                   remove alpha bytes (3x float
 *                                   operations/pixel compared with composite
 *                                   on gray background)
 *
 *  Greg's change will do this.  The reason it wasn't done before is for
 *  performance, as this increases the per-pixel operations.  If we would check
 *  in advance if the background was gray or RGB, and position the gray-to-RGB
 *  transform appropriately, then it would save a lot of work/time.
 */

#ifdef PNG_READ_GRAY_TO_RGB_SUPPORTED
   /* If gray -> RGB, do so now only if background is non-gray; else do later
    * for performance reasons
    */
   if ((png_ptr->transformations & PNG_GRAY_TO_RGB) &&
       !(png_ptr->mode & PNG_BACKGROUND_IS_GRAY))
      png_do_gray_to_rgb(&(png_ptr->row_info), png_ptr->row_buf + 1);
#endif

#ifdef PNG_READ_BACKGROUND_SUPPORTED
   if ((png_ptr->transformations & PNG_BACKGROUND) &&
      ((png_ptr->num_trans != 0 ) ||
      (png_ptr->color_type & PNG_COLOR_MASK_ALPHA)))
      png_do_background(&(png_ptr->row_info), png_ptr->row_buf + 1,
         &(png_ptr->trans_values), &(png_ptr->background)
#ifdef PNG_READ_GAMMA_SUPPORTED
         , &(png_ptr->background_1),
         png_ptr->gamma_table, png_ptr->gamma_from_1,
         png_ptr->gamma_to_1, png_ptr->gamma_16_table,
         png_ptr->gamma_16_from_1, png_ptr->gamma_16_to_1,
         png_ptr->gamma_shift
#endif
);
#endif

#ifdef PNG_READ_GAMMA_SUPPORTED
   if ((png_ptr->transformations & PNG_GAMMA) &&
#ifdef PNG_READ_BACKGROUND_SUPPORTED
       !((png_ptr->transformations & PNG_BACKGROUND) &&
       ((png_ptr->num_trans != 0) ||
       (png_ptr->color_type & PNG_COLOR_MASK_ALPHA))) &&
#endif
       (png_ptr->color_type != PNG_COLOR_TYPE_PALETTE))
      png_do_gamma(&(png_ptr->row_info), png_ptr->row_buf + 1,
          png_ptr->gamma_table, png_ptr->gamma_16_table,
          png_ptr->gamma_shift);
#endif

#ifdef PNG_READ_16_TO_8_SUPPORTED
   if (png_ptr->transformations & PNG_16_TO_8)
      png_do_chop(&(png_ptr->row_info), png_ptr->row_buf + 1);
#endif

#ifdef PNG_READ_DITHER_SUPPORTED
   if (png_ptr->transformations & PNG_DITHER)
   {
      png_do_dither((png_row_infop)&(png_ptr->row_info), png_ptr->row_buf + 1,
         png_ptr->palette_lookup, png_ptr->dither_index);
      if (png_ptr->row_info.rowbytes == (png_uint_32)0)
         png_error(png_ptr, "png_do_dither returned rowbytes=0");
   }
#endif

#ifdef PNG_READ_INVERT_SUPPORTED
   if (png_ptr->transformations & PNG_INVERT_MONO)
      png_do_invert(&(png_ptr->row_info), png_ptr->row_buf + 1);
#endif

#ifdef PNG_READ_SHIFT_SUPPORTED
   if (png_ptr->transformations & PNG_SHIFT)
      png_do_unshift(&(png_ptr->row_info), png_ptr->row_buf + 1,
         &(png_ptr->shift));
#endif

#ifdef PNG_READ_PACK_SUPPORTED
   if (png_ptr->transformations & PNG_PACK)
      png_do_unpack(&(png_ptr->row_info), png_ptr->row_buf + 1);
#endif

#ifdef PNG_READ_BGR_SUPPORTED
   if (png_ptr->transformations & PNG_BGR)
      png_do_bgr(&(png_ptr->row_info), png_ptr->row_buf + 1);
#endif

#ifdef PNG_READ_PACKSWAP_SUPPORTED
   if (png_ptr->transformations & PNG_PACKSWAP)
      png_do_packswap(&(png_ptr->row_info), png_ptr->row_buf + 1);
#endif

#ifdef PNG_READ_GRAY_TO_RGB_SUPPORTED
   /* If gray -> RGB, do so now only if we did not do so above */
   if ((png_ptr->transformations & PNG_GRAY_TO_RGB) &&
       (png_ptr->mode & PNG_BACKGROUND_IS_GRAY))
      png_do_gray_to_rgb(&(png_ptr->row_info), png_ptr->row_buf + 1);
#endif

#ifdef PNG_READ_FILLER_SUPPORTED
   if (png_ptr->transformations & PNG_FILLER)
      png_do_read_filler(&(png_ptr->row_info), png_ptr->row_buf + 1,
         (png_uint_32)png_ptr->filler, png_ptr->flags);
#endif

#ifdef PNG_READ_INVERT_ALPHA_SUPPORTED
   if (png_ptr->transformations & PNG_INVERT_ALPHA)
      png_do_read_invert_alpha(&(png_ptr->row_info), png_ptr->row_buf + 1);
#endif

#ifdef PNG_READ_SWAP_ALPHA_SUPPORTED
   if (png_ptr->transformations & PNG_SWAP_ALPHA)
      png_do_read_swap_alpha(&(png_ptr->row_info), png_ptr->row_buf + 1);
#endif

#ifdef PNG_READ_SWAP_SUPPORTED
   if (png_ptr->transformations & PNG_SWAP_BYTES)
      png_do_swap(&(png_ptr->row_info), png_ptr->row_buf + 1);
#endif

#ifdef PNG_READ_USER_TRANSFORM_SUPPORTED
   if (png_ptr->transformations & PNG_USER_TRANSFORM)
    {
      if (png_ptr->read_user_transform_fn != NULL)
         (*(png_ptr->read_user_transform_fn)) /* User read transform function */
            (png_ptr,                    /* png_ptr */
               &(png_ptr->row_info),     /* row_info: */
               /*  png_uint_32 width;       width of row */
               /*  png_uint_32 rowbytes;    number of bytes in row */
               /*  png_byte color_type;     color type of pixels */
               /*  png_byte bit_depth;      bit depth of samples */
               /*  png_byte channels;       number of channels (1-4) */
               /*  png_byte pixel_depth;    bits per pixel (depth*channels) */
               png_ptr->row_buf + 1);    /* start of pixel data for row */
#ifdef PNG_USER_TRANSFORM_PTR_SUPPORTED
      if (png_ptr->user_transform_depth)
         png_ptr->row_info.bit_depth = png_ptr->user_transform_depth;
      if (png_ptr->user_transform_channels)
         png_ptr->row_info.channels = png_ptr->user_transform_channels;
#endif
      png_ptr->row_info.pixel_depth = (png_byte)(png_ptr->row_info.bit_depth *
         png_ptr->row_info.channels);
      png_ptr->row_info.rowbytes = PNG_ROWBYTES(png_ptr->row_info.pixel_depth,
         png_ptr->row_info.width);
   }
#endif

}
