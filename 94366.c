 png_read_finish_row(png_structp png_ptr)
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
 #endif /* PNG_READ_INTERLACING_SUPPORTED */
 
    png_debug(1, "in png_read_finish_row");
   png_ptr->row_number++;
   if (png_ptr->row_number < png_ptr->num_rows)
      return;

#ifdef PNG_READ_INTERLACING_SUPPORTED
   if (png_ptr->interlaced)
   {
      png_ptr->row_number = 0;
      png_memset_check(png_ptr, png_ptr->prev_row, 0,
         png_ptr->rowbytes + 1);
      do
      {
         png_ptr->pass++;
         if (png_ptr->pass >= 7)
            break;
         png_ptr->iwidth = (png_ptr->width +
            png_pass_inc[png_ptr->pass] - 1 -
            png_pass_start[png_ptr->pass]) /
            png_pass_inc[png_ptr->pass];

         if (!(png_ptr->transformations & PNG_INTERLACE))
         {
            png_ptr->num_rows = (png_ptr->height +
               png_pass_yinc[png_ptr->pass] - 1 -
               png_pass_ystart[png_ptr->pass]) /
               png_pass_yinc[png_ptr->pass];
            if (!(png_ptr->num_rows))
               continue;
         }
         else  /* if (png_ptr->transformations & PNG_INTERLACE) */
            break;
      } while (png_ptr->iwidth == 0);

      if (png_ptr->pass < 7)
         return;
   }
#endif /* PNG_READ_INTERLACING_SUPPORTED */

   if (!(png_ptr->flags & PNG_FLAG_ZLIB_FINISHED))
   {
#ifdef PNG_USE_LOCAL_ARRAYS
      PNG_CONST PNG_IDAT;
#endif
      char extra;
      int ret;

      png_ptr->zstream.next_out = (Byte *)&extra;
      png_ptr->zstream.avail_out = (uInt)1;
      for (;;)
      {
         if (!(png_ptr->zstream.avail_in))
         {
            while (!png_ptr->idat_size)
            {
               png_byte chunk_length[4];

               png_crc_finish(png_ptr, 0);

               png_read_data(png_ptr, chunk_length, 4);
               png_ptr->idat_size = png_get_uint_31(png_ptr, chunk_length);
               png_reset_crc(png_ptr);
               png_crc_read(png_ptr, png_ptr->chunk_name, 4);
               if (png_memcmp(png_ptr->chunk_name, png_IDAT, 4))
                  png_error(png_ptr, "Not enough image data");

            }
            png_ptr->zstream.avail_in = (uInt)png_ptr->zbuf_size;
            png_ptr->zstream.next_in = png_ptr->zbuf;
            if (png_ptr->zbuf_size > png_ptr->idat_size)
               png_ptr->zstream.avail_in = (uInt)png_ptr->idat_size;
            png_crc_read(png_ptr, png_ptr->zbuf, png_ptr->zstream.avail_in);
            png_ptr->idat_size -= png_ptr->zstream.avail_in;
         }
         ret = inflate(&png_ptr->zstream, Z_PARTIAL_FLUSH);
         if (ret == Z_STREAM_END)
         {
            if (!(png_ptr->zstream.avail_out) || png_ptr->zstream.avail_in ||
               png_ptr->idat_size)
               png_warning(png_ptr, "Extra compressed data.");
            png_ptr->mode |= PNG_AFTER_IDAT;
            png_ptr->flags |= PNG_FLAG_ZLIB_FINISHED;
            break;
         }
         if (ret != Z_OK)
            png_error(png_ptr, png_ptr->zstream.msg ? png_ptr->zstream.msg :
                      "Decompression Error");

         if (!(png_ptr->zstream.avail_out))
         {
            png_warning(png_ptr, "Extra compressed data.");
            png_ptr->mode |= PNG_AFTER_IDAT;
            png_ptr->flags |= PNG_FLAG_ZLIB_FINISHED;
            break;
         }

      }
      png_ptr->zstream.avail_out = 0;
   }

   if (png_ptr->idat_size || png_ptr->zstream.avail_in)
      png_warning(png_ptr, "Extra compression data.");

   inflateReset(&png_ptr->zstream);

   png_ptr->mode |= PNG_AFTER_IDAT;
}
