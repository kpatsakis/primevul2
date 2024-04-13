 png_read_start_row(png_structp png_ptr)
 {
 #ifdef PNG_READ_INTERLACING_SUPPORTED
#ifndef PNG_USE_GLOBAL_ARRAYS
    /* Arrays to facilitate easy interlacing - use pass (0 - 6) as index */
 
    /* Start of interlace block */
   PNG_CONST int png_pass_start[7] = {0, 4, 0, 2, 0, 1, 0};

   /* Offset to next interlace block */
   PNG_CONST int png_pass_inc[7] = {8, 8, 4, 4, 2, 2, 1};

   /* Start of interlace block in the y direction */
   PNG_CONST int png_pass_ystart[7] = {0, 0, 4, 0, 2, 0, 1};
 
    /* Offset to next interlace block in the y direction */
    PNG_CONST int png_pass_yinc[7] = {8, 8, 8, 4, 4, 2, 2};
#endif
 #endif
 
    int max_pixel_depth;
   png_size_t row_bytes;

   png_debug(1, "in png_read_start_row");
   png_ptr->zstream.avail_in = 0;
   png_init_read_transformations(png_ptr);
#ifdef PNG_READ_INTERLACING_SUPPORTED
   if (png_ptr->interlaced)
   {
      if (!(png_ptr->transformations & PNG_INTERLACE))
         png_ptr->num_rows = (png_ptr->height + png_pass_yinc[0] - 1 -
            png_pass_ystart[0]) / png_pass_yinc[0];
      else
         png_ptr->num_rows = png_ptr->height;

      png_ptr->iwidth = (png_ptr->width +
         png_pass_inc[png_ptr->pass] - 1 -
         png_pass_start[png_ptr->pass]) /
         png_pass_inc[png_ptr->pass];
   }
   else
#endif /* PNG_READ_INTERLACING_SUPPORTED */
   {
      png_ptr->num_rows = png_ptr->height;
      png_ptr->iwidth = png_ptr->width;
   }
   max_pixel_depth = png_ptr->pixel_depth;

#ifdef PNG_READ_PACK_SUPPORTED
   if ((png_ptr->transformations & PNG_PACK) && png_ptr->bit_depth < 8)
      max_pixel_depth = 8;
#endif

#ifdef PNG_READ_EXPAND_SUPPORTED
   if (png_ptr->transformations & PNG_EXPAND)
   {
      if (png_ptr->color_type == PNG_COLOR_TYPE_PALETTE)
      {
         if (png_ptr->num_trans)
            max_pixel_depth = 32;
         else
            max_pixel_depth = 24;
      }
      else if (png_ptr->color_type == PNG_COLOR_TYPE_GRAY)
      {
         if (max_pixel_depth < 8)
            max_pixel_depth = 8;
         if (png_ptr->num_trans)
            max_pixel_depth *= 2;
      }
      else if (png_ptr->color_type == PNG_COLOR_TYPE_RGB)
      {
         if (png_ptr->num_trans)
         {
            max_pixel_depth *= 4;
            max_pixel_depth /= 3;
         }
      }
   }
#endif

#ifdef PNG_READ_FILLER_SUPPORTED
   if (png_ptr->transformations & (PNG_FILLER))
   {
      if (png_ptr->color_type == PNG_COLOR_TYPE_PALETTE)
         max_pixel_depth = 32;
      else if (png_ptr->color_type == PNG_COLOR_TYPE_GRAY)
      {
         if (max_pixel_depth <= 8)
            max_pixel_depth = 16;
         else
            max_pixel_depth = 32;
      }
      else if (png_ptr->color_type == PNG_COLOR_TYPE_RGB)
      {
         if (max_pixel_depth <= 32)
            max_pixel_depth = 32;
         else
            max_pixel_depth = 64;
      }
   }
#endif

#ifdef PNG_READ_GRAY_TO_RGB_SUPPORTED
   if (png_ptr->transformations & PNG_GRAY_TO_RGB)
   {
      if (
#ifdef PNG_READ_EXPAND_SUPPORTED
        (png_ptr->num_trans && (png_ptr->transformations & PNG_EXPAND)) ||
#endif
#ifdef PNG_READ_FILLER_SUPPORTED
        (png_ptr->transformations & (PNG_FILLER)) ||
#endif
        png_ptr->color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
      {
         if (max_pixel_depth <= 16)
            max_pixel_depth = 32;
         else
            max_pixel_depth = 64;
      }
      else
      {
         if (max_pixel_depth <= 8)
           {
             if (png_ptr->color_type == PNG_COLOR_TYPE_RGB_ALPHA)
               max_pixel_depth = 32;
             else
               max_pixel_depth = 24;
           }
         else if (png_ptr->color_type == PNG_COLOR_TYPE_RGB_ALPHA)
            max_pixel_depth = 64;
         else
            max_pixel_depth = 48;
      }
   }
#endif

#if defined(PNG_READ_USER_TRANSFORM_SUPPORTED) && \
defined(PNG_USER_TRANSFORM_PTR_SUPPORTED)
   if (png_ptr->transformations & PNG_USER_TRANSFORM)
     {
       int user_pixel_depth = png_ptr->user_transform_depth*
         png_ptr->user_transform_channels;
       if (user_pixel_depth > max_pixel_depth)
         max_pixel_depth=user_pixel_depth;
     }
#endif

   /* Align the width on the next larger 8 pixels.  Mainly used
    * for interlacing
    */
   row_bytes = ((png_ptr->width + 7) & ~((png_uint_32)7));
   /* Calculate the maximum bytes needed, adding a byte and a pixel
    * for safety's sake
    */
   row_bytes = PNG_ROWBYTES(max_pixel_depth, row_bytes) +
      1 + ((max_pixel_depth + 7) >> 3);
#ifdef PNG_MAX_MALLOC_64K
   if (row_bytes > (png_uint_32)65536L)
      png_error(png_ptr, "This image requires a row greater than 64KB");
#endif

   if (row_bytes + 64 > png_ptr->old_big_row_buf_size)
   {
     png_free(png_ptr, png_ptr->big_row_buf);
     if (png_ptr->interlaced)
        png_ptr->big_row_buf = (png_bytep)png_calloc(png_ptr,
            row_bytes + 64);
     else
        png_ptr->big_row_buf = (png_bytep)png_malloc(png_ptr,
            row_bytes + 64);
     png_ptr->old_big_row_buf_size = row_bytes + 64;

     /* Use 32 bytes of padding before and after row_buf. */
     png_ptr->row_buf = png_ptr->big_row_buf + 32;
     png_ptr->old_big_row_buf_size = row_bytes + 64;
   }

#ifdef PNG_MAX_MALLOC_64K
   if ((png_uint_32)row_bytes + 1 > (png_uint_32)65536L)
      png_error(png_ptr, "This image requires a row greater than 64KB");
#endif
   if ((png_uint_32)row_bytes > (png_uint_32)(PNG_SIZE_MAX - 1))
      png_error(png_ptr, "Row has too many bytes to allocate in memory.");

   if (row_bytes + 1 > png_ptr->old_prev_row_size)
   {
      png_free(png_ptr, png_ptr->prev_row);
      png_ptr->prev_row = (png_bytep)png_malloc(png_ptr, (png_uint_32)(
        row_bytes + 1));
      png_memset_check(png_ptr, png_ptr->prev_row, 0, row_bytes + 1);
      png_ptr->old_prev_row_size = row_bytes + 1;
   }

   png_ptr->rowbytes = row_bytes;

   png_debug1(3, "width = %lu,", png_ptr->width);
   png_debug1(3, "height = %lu,", png_ptr->height);
   png_debug1(3, "iwidth = %lu,", png_ptr->iwidth);
   png_debug1(3, "num_rows = %lu,", png_ptr->num_rows);
   png_debug1(3, "rowbytes = %lu,", png_ptr->rowbytes);
   png_debug1(3, "irowbytes = %lu",
       PNG_ROWBYTES(png_ptr->pixel_depth, png_ptr->iwidth) + 1);

   png_ptr->flags |= PNG_FLAG_ROW_INIT;
}
