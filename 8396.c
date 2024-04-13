void read_pcm_samples_internal(thread_context* tctx, int x0, int y0, int log2CbSize,
                               int cIdx, bitreader& br)
{
  const seq_parameter_set& sps = tctx->img->get_sps();

  int nPcmBits;
  int bitDepth;

  int w = 1<<log2CbSize;
  int h = 1<<log2CbSize;

  if (cIdx>0) {
    w /= sps.SubWidthC;
    h /= sps.SubHeightC;

    x0 /= sps.SubWidthC;
    y0 /= sps.SubHeightC;

    nPcmBits = sps.pcm_sample_bit_depth_chroma;
    bitDepth = sps.BitDepth_C;
  }
  else {
    nPcmBits = sps.pcm_sample_bit_depth_luma;
    bitDepth = sps.BitDepth_Y;
  }

  pixel_t* ptr;
  int stride;
  ptr    = tctx->img->get_image_plane_at_pos_NEW<pixel_t>(cIdx,x0,y0);
  stride = tctx->img->get_image_stride(cIdx);

  int shift = bitDepth - nPcmBits;

  // a shift < 0 may result when the SPS sequence header is broken
  if (shift < 0) {
    shift=0;
  }

  for (int y=0;y<h;y++)
    for (int x=0;x<w;x++)
      {
        int value = get_bits(&br, nPcmBits);
        ptr[y*stride+x] = value << shift;
      }
}