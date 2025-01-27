png_process_IDAT_data(png_structp png_ptr, png_bytep buffer,
   png_size_t buffer_length)
{
   /* The caller checks for a non-zero buffer length. */
   if (!(buffer_length > 0) || buffer == NULL)
      png_error(png_ptr, "No IDAT data (internal error)");

   /* This routine must process all the data it has been given
    * before returning, calling the row callback as required to
    * handle the uncompressed results.
    */
   png_ptr->zstream.next_in = buffer;
   png_ptr->zstream.avail_in = (uInt)buffer_length;

   /* Keep going until the decompressed data is all processed
    * or the stream marked as finished.
    */
   while (png_ptr->zstream.avail_in > 0 &&
	  !(png_ptr->flags & PNG_FLAG_ZLIB_FINISHED))
   {
      int ret;

      /* We have data for zlib, but we must check that zlib
       * has somewhere to put the results.  It doesn't matter
       * if we don't expect any results -- it may be the input
       * data is just the LZ end code.
       */
      if (!(png_ptr->zstream.avail_out > 0))
      {
         png_ptr->zstream.avail_out =
             (uInt) PNG_ROWBYTES(png_ptr->pixel_depth,
             png_ptr->iwidth) + 1;
         png_ptr->zstream.next_out = png_ptr->row_buf;
      }

      /* Using Z_SYNC_FLUSH here means that an unterminated
       * LZ stream can still be handled (a stream with a missing
       * end code), otherwise (Z_NO_FLUSH) a future zlib
       * implementation might defer output and, therefore,
       * change the current behavior.  (See comments in inflate.c
       * for why this doesn't happen at present with zlib 1.2.5.)
       */
      ret = inflate(&png_ptr->zstream, Z_SYNC_FLUSH);

      /* Check for any failure before proceeding. */
      if (ret != Z_OK && ret != Z_STREAM_END)
      {
	 /* Terminate the decompression. */
	 png_ptr->flags |= PNG_FLAG_ZLIB_FINISHED;

         /* This may be a truncated stream (missing or
	  * damaged end code).  Treat that as a warning.
	  */
         if (png_ptr->row_number >= png_ptr->num_rows ||
	     png_ptr->pass > 6)
	    png_warning(png_ptr, "Truncated compressed data in IDAT");
	 else
	    png_error(png_ptr, "Decompression error in IDAT");

	 /* Skip the check on unprocessed input */
         return;
      }

      /* Did inflate output any data? */
      if (png_ptr->zstream.next_out != png_ptr->row_buf)
      {
	 /* Is this unexpected data after the last row?
	  * If it is, artificially terminate the LZ output
	  * here.
	  */
         if (png_ptr->row_number >= png_ptr->num_rows ||
	     png_ptr->pass > 6)
         {
	    /* Extra data. */
	    png_warning(png_ptr, "Extra compressed data in IDAT");
            png_ptr->flags |= PNG_FLAG_ZLIB_FINISHED;
	    /* Do no more processing; skip the unprocessed
	     * input check below.
	     */
            return;
	 }

	 /* Do we have a complete row? */
	 if (png_ptr->zstream.avail_out == 0)
	    png_push_process_row(png_ptr);
      }

      /* And check for the end of the stream. */
      if (ret == Z_STREAM_END)
	 png_ptr->flags |= PNG_FLAG_ZLIB_FINISHED;
   }

   /* All the data should have been processed, if anything
    * is left at this point we have bytes of IDAT data
    * after the zlib end code.
    */
   if (png_ptr->zstream.avail_in > 0)
      png_warning(png_ptr, "Extra compression data");
}
