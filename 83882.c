png_set_sPLT(png_structp png_ptr,
             png_infop info_ptr, png_sPLT_tp entries, int nentries)
/*
 *  entries        - array of png_sPLT_t structures
 *                   to be added to the list of palettes
 *                   in the info structure.
 *  nentries       - number of palette structures to be
 *                   added.
 */
{
   png_sPLT_tp np;
   int i;

   if (png_ptr == NULL || info_ptr == NULL)
      return;

   np = (png_sPLT_tp)png_malloc_warn(png_ptr,
       (info_ptr->splt_palettes_num + nentries) *
        (png_uint_32)png_sizeof(png_sPLT_t));
   if (np == NULL)
   {
      png_warning(png_ptr, "No memory for sPLT palettes.");
      return;
   }

   png_memcpy(np, info_ptr->splt_palettes,
       info_ptr->splt_palettes_num * png_sizeof(png_sPLT_t));
   png_free(png_ptr, info_ptr->splt_palettes);
   info_ptr->splt_palettes=NULL;

   for (i = 0; i < nentries; i++)
   {
      png_sPLT_tp to = np + info_ptr->splt_palettes_num + i;
      png_sPLT_tp from = entries + i;
      png_uint_32 length;

      length = png_strlen(from->name) + 1;
      to->name = (png_charp)png_malloc_warn(png_ptr, length);
      if (to->name == NULL)
      {
         png_warning(png_ptr,
           "Out of memory while processing sPLT chunk");
         continue;
      }
      png_memcpy(to->name, from->name, length);
      to->entries = (png_sPLT_entryp)png_malloc_warn(png_ptr,
          (png_uint_32)(from->nentries * png_sizeof(png_sPLT_entry)));
      if (to->entries == NULL)
      {
         png_warning(png_ptr,
           "Out of memory while processing sPLT chunk");
         png_free(png_ptr, to->name);
         to->name = NULL;
         continue;
      }
      png_memcpy(to->entries, from->entries,
          from->nentries * png_sizeof(png_sPLT_entry));
      to->nentries = from->nentries;
      to->depth = from->depth;
   }

   info_ptr->splt_palettes = np;
   info_ptr->splt_palettes_num += nentries;
   info_ptr->valid |= PNG_INFO_sPLT;
#ifdef PNG_FREE_ME_SUPPORTED
   info_ptr->free_me |= PNG_FREE_SPLT;
#endif
}
