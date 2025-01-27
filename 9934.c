get_word_rgb_row (j_compress_ptr cinfo, cjpeg_source_ptr sinfo)
/* This version is for reading raw-word-format PPM files with any maxval */
{
  ppm_source_ptr source = (ppm_source_ptr) sinfo;
  register JSAMPROW ptr;
  register U_CHAR * bufferptr;
  register JSAMPLE *rescale = source->rescale;
  JDIMENSION col;

  if (! ReadOK(source->pub.input_file, source->iobuffer, source->buffer_width))
    ERREXIT(cinfo, JERR_INPUT_EOF);
  ptr = source->pub.buffer[0];
  bufferptr = source->iobuffer;
  for (col = cinfo->image_width; col > 0; col--) {
    register int temp;
    temp  = UCH(*bufferptr++) << 8;
    temp |= UCH(*bufferptr++);
    *ptr++ = rescale[temp];
    temp  = UCH(*bufferptr++) << 8;
    temp |= UCH(*bufferptr++);
    *ptr++ = rescale[temp];
    temp  = UCH(*bufferptr++) << 8;
    temp |= UCH(*bufferptr++);
    *ptr++ = rescale[temp];
  }
  return 1;
}