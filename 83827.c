png_set_read_status_fn(png_structp png_ptr, png_read_status_ptr read_row_fn)
{
   if (png_ptr == NULL)
      return;
   png_ptr->read_row_fn = read_row_fn;
}
