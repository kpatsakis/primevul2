png_info_init(png_infop info_ptr)
{
   /* We only come here via pre-1.0.12-compiled applications */
   png_info_init_3(&info_ptr, 0);
}
