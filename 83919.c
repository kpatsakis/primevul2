png_write_chunk_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
   /* Write the data, and run the CRC over it */
   if (png_ptr == NULL)
      return;
   if (data != NULL && length > 0)
   {
      png_write_data(png_ptr, data, length);
      /* Update the CRC after writing the data,
       * in case that the user I/O routine alters it.
       */
      png_calculate_crc(png_ptr, data, length);
   }
}
