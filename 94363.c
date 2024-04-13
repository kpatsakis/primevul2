 png_handle_PLTE(png_structp png_ptr, png_infop info_ptr, png_uint_32 length)
 {
    png_color palette[PNG_MAX_PALETTE_LENGTH];
   int max_palette_length, num, i;
 #ifdef PNG_POINTER_INDEXING_SUPPORTED
    png_colorp pal_ptr;
 #endif

   png_debug(1, "in png_handle_PLTE");

   if (!(png_ptr->mode & PNG_HAVE_IHDR))
      png_error(png_ptr, "Missing IHDR before PLTE");

   else if (png_ptr->mode & PNG_HAVE_IDAT)
   {
      png_warning(png_ptr, "Invalid PLTE after IDAT");
      png_crc_finish(png_ptr, length);
      return;
   }

   else if (png_ptr->mode & PNG_HAVE_PLTE)
      png_error(png_ptr, "Duplicate PLTE chunk");

   png_ptr->mode |= PNG_HAVE_PLTE;

   if (!(png_ptr->color_type&PNG_COLOR_MASK_COLOR))
   {
      png_warning(png_ptr,
        "Ignoring PLTE chunk in grayscale PNG");
      png_crc_finish(png_ptr, length);
      return;
   }
#ifndef PNG_READ_OPT_PLTE_SUPPORTED
   if (png_ptr->color_type != PNG_COLOR_TYPE_PALETTE)
   {
      png_crc_finish(png_ptr, length);
      return;
   }
#endif

   if (length > 3*PNG_MAX_PALETTE_LENGTH || length % 3)
   {
      if (png_ptr->color_type != PNG_COLOR_TYPE_PALETTE)
      {
         png_warning(png_ptr, "Invalid palette chunk");
         png_crc_finish(png_ptr, length);
         return;
      }

      else
      {
         png_error(png_ptr, "Invalid palette chunk");
       }
    }
 
   /* The cast is safe because 'length' is less than 3*PNG_MAX_PALETTE_LENGTH */
    num = (int)length / 3;
 
   /* If the palette has 256 or fewer entries but is too large for the bit
    * depth, we don't issue an error, to preserve the behavior of previous
    * libpng versions. We silently truncate the unused extra palette entries
    * here.
    */
   if (png_ptr->color_type == PNG_COLOR_TYPE_PALETTE)
      max_palette_length = (1 << png_ptr->bit_depth);
   else
      max_palette_length = PNG_MAX_PALETTE_LENGTH;

   if (num > max_palette_length)
      num = max_palette_length;

 #ifdef PNG_POINTER_INDEXING_SUPPORTED
    for (i = 0, pal_ptr = palette; i < num; i++, pal_ptr++)
    {
      png_byte buf[3];

      png_crc_read(png_ptr, buf, 3);
      pal_ptr->red = buf[0];
      pal_ptr->green = buf[1];
      pal_ptr->blue = buf[2];
   }
#else
   for (i = 0; i < num; i++)
   {
      png_byte buf[3];

      png_crc_read(png_ptr, buf, 3);
      /* Don't depend upon png_color being any order */
      palette[i].red = buf[0];
      palette[i].green = buf[1];
      palette[i].blue = buf[2];
   }
#endif

   /* If we actually NEED the PLTE chunk (ie for a paletted image), we do
    * whatever the normal CRC configuration tells us.  However, if we
    * have an RGB image, the PLTE can be considered ancillary, so
    * we will act as though it is.
    */
#ifndef PNG_READ_OPT_PLTE_SUPPORTED
    if (png_ptr->color_type == PNG_COLOR_TYPE_PALETTE)
 #endif
    {
      png_crc_finish(png_ptr, (int) length - num * 3);
    }
 #ifndef PNG_READ_OPT_PLTE_SUPPORTED
    else if (png_crc_error(png_ptr))  /* Only if we have a CRC error */
   {
      /* If we don't want to use the data from an ancillary chunk,
         we have two options: an error abort, or a warning and we
         ignore the data in this chunk (which should be OK, since
         it's considered ancillary for a RGB or RGBA image). */
      if (!(png_ptr->flags & PNG_FLAG_CRC_ANCILLARY_USE))
      {
         if (png_ptr->flags & PNG_FLAG_CRC_ANCILLARY_NOWARN)
         {
            png_chunk_error(png_ptr, "CRC error");
         }
         else
         {
            png_chunk_warning(png_ptr, "CRC error");
            return;
         }
      }
      /* Otherwise, we (optionally) emit a warning and use the chunk. */
      else if (!(png_ptr->flags & PNG_FLAG_CRC_ANCILLARY_NOWARN))
      {
         png_chunk_warning(png_ptr, "CRC error");
      }
   }
#endif

   png_set_PLTE(png_ptr, info_ptr, palette, num);

#ifdef PNG_READ_tRNS_SUPPORTED
   if (png_ptr->color_type == PNG_COLOR_TYPE_PALETTE)
   {
      if (info_ptr != NULL && (info_ptr->valid & PNG_INFO_tRNS))
      {
         if (png_ptr->num_trans > (png_uint_16)num)
         {
            png_warning(png_ptr, "Truncating incorrect tRNS chunk length");
            png_ptr->num_trans = (png_uint_16)num;
         }
         if (info_ptr->num_trans > (png_uint_16)num)
         {
            png_warning(png_ptr, "Truncating incorrect info tRNS chunk length");
            info_ptr->num_trans = (png_uint_16)num;
         }
      }
   }
#endif

}
