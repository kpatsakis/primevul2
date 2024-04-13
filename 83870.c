png_set_iCCP(png_structp png_ptr, png_infop info_ptr,
             png_charp name, int compression_type,
             png_charp profile, png_uint_32 proflen)
{
   png_charp new_iccp_name;
   png_charp new_iccp_profile;
   png_uint_32 length;

   png_debug1(1, "in %s storage function", "iCCP");

   if (png_ptr == NULL || info_ptr == NULL || name == NULL || profile == NULL)
      return;

   length = png_strlen(name)+1;
   new_iccp_name = (png_charp)png_malloc_warn(png_ptr, length);
   if (new_iccp_name == NULL)
   {
        png_warning(png_ptr, "Insufficient memory to process iCCP chunk.");
      return;
   }
   png_memcpy(new_iccp_name, name, length);
   new_iccp_profile = (png_charp)png_malloc_warn(png_ptr, proflen);
   if (new_iccp_profile == NULL)
   {
      png_free (png_ptr, new_iccp_name);
      png_warning(png_ptr,
          "Insufficient memory to process iCCP profile.");
      return;
   }
   png_memcpy(new_iccp_profile, profile, (png_size_t)proflen);

   png_free_data(png_ptr, info_ptr, PNG_FREE_ICCP, 0);

   info_ptr->iccp_proflen = proflen;
   info_ptr->iccp_name = new_iccp_name;
   info_ptr->iccp_profile = new_iccp_profile;
   /* Compression is always zero but is here so the API and info structure
    * does not have to change if we introduce multiple compression types
    */
   info_ptr->iccp_compression = (png_byte)compression_type;
#ifdef PNG_FREE_ME_SUPPORTED
   info_ptr->free_me |= PNG_FREE_ICCP;
#endif
   info_ptr->valid |= PNG_INFO_iCCP;
}
