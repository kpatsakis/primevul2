png_decompress_chunk(png_structp png_ptr, int comp_type,
    png_size_t chunklength,
    png_size_t prefix_size, png_size_t *newlength)
{
   /* The caller should guarantee this */
   if (prefix_size > chunklength)
   {
      /* The recovery is to delete the chunk. */
      png_warning(png_ptr, "invalid chunklength");
      prefix_size = 0; /* To delete everything */
   }

   else if (comp_type == PNG_COMPRESSION_TYPE_BASE)
   {
      png_size_t expanded_size = png_inflate(png_ptr,
                (png_bytep)(png_ptr->chunkdata + prefix_size),
                chunklength - prefix_size,
                0/*output*/, 0/*output size*/);

      /* Now check the limits on this chunk - if the limit fails the
       * compressed data will be removed, the prefix will remain.
       */
      if (prefix_size >= (~(png_size_t)0) - 1 ||
         expanded_size >= (~(png_size_t)0) - 1 - prefix_size
#ifdef PNG_USER_CHUNK_MALLOC_MAX
         || ((PNG_USER_CHUNK_MALLOC_MAX > 0) &&
          prefix_size + expanded_size >= PNG_USER_CHUNK_MALLOC_MAX - 1)
#endif
          )
         png_warning(png_ptr, "Exceeded size limit while expanding chunk");

      /* If the size is zero either there was an error and a message
       * has already been output (warning) or the size really is zero
       * and we have nothing to do - the code will exit through the
       * error case below.
       */
      else if (expanded_size > 0)
      {
         /* Success (maybe) - really uncompress the chunk. */
         png_size_t new_size = 0;

         png_charp text = png_malloc_warn(png_ptr,
             prefix_size + expanded_size + 1);

         if (text != NULL)
         {
            png_memcpy(text, png_ptr->chunkdata, prefix_size);
            new_size = png_inflate(png_ptr,
                (png_bytep)(png_ptr->chunkdata + prefix_size),
                chunklength - prefix_size,
                (png_bytep)(text + prefix_size), expanded_size);
            text[prefix_size + expanded_size] = 0; /* just in case */

            if (new_size == expanded_size)
            {
               png_free(png_ptr, png_ptr->chunkdata);
               png_ptr->chunkdata = text;
               *newlength = prefix_size + expanded_size;
               return; /* The success return! */
            }

            png_warning(png_ptr, "png_inflate logic error");
            png_free(png_ptr, text);
         }
         else
          png_warning(png_ptr, "Not enough memory to decompress chunk.");
      }
   }

   else /* if (comp_type != PNG_COMPRESSION_TYPE_BASE) */
   {
#if defined(PNG_STDIO_SUPPORTED) && !defined(_WIN32_WCE)
      char umsg[50];

      png_snprintf(umsg, sizeof umsg, "Unknown zTXt compression type %d",
          comp_type);
      png_warning(png_ptr, umsg);
#else
      png_warning(png_ptr, "Unknown zTXt compression type");
#endif

      /* The recovery is to simply drop the data. */
   }

   /* Generic error return - leave the prefix, delete the compressed
    * data, reallocate the chunkdata to remove the potentially large
    * amount of compressed data.
    */
   {
      png_charp text = png_malloc_warn(png_ptr, prefix_size + 1);
      if (text != NULL)
      {
         if (prefix_size > 0)
            png_memcpy(text, png_ptr->chunkdata, prefix_size);
         png_free(png_ptr, png_ptr->chunkdata);
         png_ptr->chunkdata = text;

         /* This is an extra zero in the 'uncompressed' part. */
         *(png_ptr->chunkdata + prefix_size) = 0x00;
      }
      /* Ignore a malloc error here - it is safe. */
   }

   *newlength = prefix_size;
}
