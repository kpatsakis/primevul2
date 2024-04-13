png_write_compressed_data_out(png_structp png_ptr, compression_state *comp)
{
   int i;

   /* Handle the no-compression case */
   if (comp->input)
   {
      png_write_chunk_data(png_ptr, (png_bytep)comp->input,
                            (png_size_t)comp->input_len);
      return;
   }

   /* Write saved output buffers, if any */
   for (i = 0; i < comp->num_output_ptr; i++)
   {
      png_write_chunk_data(png_ptr, (png_bytep)comp->output_ptr[i],
         (png_size_t)png_ptr->zbuf_size);
      png_free(png_ptr, comp->output_ptr[i]);
       comp->output_ptr[i]=NULL;
   }
   if (comp->max_output_ptr != 0)
      png_free(png_ptr, comp->output_ptr);
       comp->output_ptr=NULL;
   /* Write anything left in zbuf */
   if (png_ptr->zstream.avail_out < (png_uint_32)png_ptr->zbuf_size)
      png_write_chunk_data(png_ptr, png_ptr->zbuf,
         (png_size_t)(png_ptr->zbuf_size - png_ptr->zstream.avail_out));

   /* Reset zlib for another zTXt/iTXt or image data */
   deflateReset(&png_ptr->zstream);
   png_ptr->zstream.data_type = Z_BINARY;
}
