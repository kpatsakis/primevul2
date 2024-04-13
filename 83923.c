png_write_filtered_row(png_structp png_ptr, png_bytep filtered_row)
{
   png_debug(1, "in png_write_filtered_row");

   png_debug1(2, "filter = %d", filtered_row[0]);
   /* Set up the zlib input buffer */

   png_ptr->zstream.next_in = filtered_row;
   png_ptr->zstream.avail_in = (uInt)png_ptr->row_info.rowbytes + 1;
   /* Repeat until we have compressed all the data */
   do
   {
      int ret; /* Return of zlib */

      /* Compress the data */
      ret = deflate(&png_ptr->zstream, Z_NO_FLUSH);
      /* Check for compression errors */
      if (ret != Z_OK)
      {
         if (png_ptr->zstream.msg != NULL)
            png_error(png_ptr, png_ptr->zstream.msg);
         else
            png_error(png_ptr, "zlib error");
      }

      /* See if it is time to write another IDAT */
      if (!(png_ptr->zstream.avail_out))
      {
         /* Write the IDAT and reset the zlib output buffer */
         png_write_IDAT(png_ptr, png_ptr->zbuf, png_ptr->zbuf_size);
         png_ptr->zstream.next_out = png_ptr->zbuf;
         png_ptr->zstream.avail_out = (uInt)png_ptr->zbuf_size;
      }
   /* Repeat until all data has been compressed */
   } while (png_ptr->zstream.avail_in);

   /* Swap the current and previous rows */
   if (png_ptr->prev_row != NULL)
   {
      png_bytep tptr;

      tptr = png_ptr->prev_row;
      png_ptr->prev_row = png_ptr->row_buf;
      png_ptr->row_buf = tptr;
   }

   /* Finish row - updates counters and flushes zlib if last row */
   png_write_finish_row(png_ptr);

#ifdef PNG_WRITE_FLUSH_SUPPORTED
   png_ptr->flush_rows++;

   if (png_ptr->flush_dist > 0 &&
       png_ptr->flush_rows >= png_ptr->flush_dist)
   {
      png_write_flush(png_ptr);
   }
#endif
}
