png_get_header_ver(png_structp png_ptr)
{
   /* Version of *.h files used when building libpng */
   PNG_UNUSED(png_ptr)  /* Silence compiler warning about unused png_ptr */
   return ((png_charp) PNG_LIBPNG_VER_STRING);
}
