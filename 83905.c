png_write_init(png_structp png_ptr)
{
   /* We only come here via pre-1.0.7-compiled applications */
   png_write_init_2(png_ptr, "1.0.6 or earlier", 0, 0);
}
