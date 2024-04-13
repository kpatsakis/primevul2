png_permit_empty_plte (png_structp png_ptr, int empty_plte_permitted)
{
   /* This function is deprecated in favor of png_permit_mng_features()
      and will be removed from libpng-1.3.0 */

   png_debug(1, "in png_permit_empty_plte, DEPRECATED.");

   if (png_ptr == NULL)
      return;
   png_ptr->mng_features_permitted = (png_byte)
     ((png_ptr->mng_features_permitted & (~PNG_FLAG_MNG_EMPTY_PLTE)) |
     ((empty_plte_permitted & PNG_FLAG_MNG_EMPTY_PLTE)));
}
