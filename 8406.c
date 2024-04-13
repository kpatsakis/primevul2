void read_coding_quadtree(thread_context* tctx,
                          int x0, int y0,
                          int log2CbSize,
                          int ctDepth)
{
  logtrace(LogSlice,"- read_coding_quadtree %d;%d cbsize:%d depth:%d POC:%d\n",x0,y0,1<<log2CbSize,ctDepth,tctx->img->PicOrderCntVal);

  de265_image* img = tctx->img;
  const seq_parameter_set& sps = img->get_sps();
  const pic_parameter_set& pps = img->get_pps();

  int split_flag;

  // We only send a split flag if CU is larger than minimum size and
  // completely contained within the image area.
  // If it is partly outside the image area and not at minimum size,
  // it is split. If already at minimum size, it is not split further.
  if (x0+(1<<log2CbSize) <= sps.pic_width_in_luma_samples &&
      y0+(1<<log2CbSize) <= sps.pic_height_in_luma_samples &&
      log2CbSize > sps.Log2MinCbSizeY) {
    split_flag = decode_split_cu_flag(tctx, x0,y0, ctDepth);
  } else {
    if (log2CbSize > sps.Log2MinCbSizeY) { split_flag=1; }
    else                                  { split_flag=0; }
  }


  if (pps.cu_qp_delta_enabled_flag &&
      log2CbSize >= pps.Log2MinCuQpDeltaSize)
    {
      tctx->IsCuQpDeltaCoded = 0;
      tctx->CuQpDelta = 0;
    }
  else
    {
      // shdr->CuQpDelta = 0; // TODO check: is this the right place to set to default value ?
    }


  if (tctx->shdr->cu_chroma_qp_offset_enabled_flag &&
      log2CbSize >= pps.Log2MinCuChromaQpOffsetSize) {
    tctx->IsCuChromaQpOffsetCoded = 0;
  }

  if (split_flag) {
    int x1 = x0 + (1<<(log2CbSize-1));
    int y1 = y0 + (1<<(log2CbSize-1));

    read_coding_quadtree(tctx,x0,y0, log2CbSize-1, ctDepth+1);

    if (x1<sps.pic_width_in_luma_samples)
      read_coding_quadtree(tctx,x1,y0, log2CbSize-1, ctDepth+1);

    if (y1<sps.pic_height_in_luma_samples)
      read_coding_quadtree(tctx,x0,y1, log2CbSize-1, ctDepth+1);

    if (x1<sps.pic_width_in_luma_samples &&
        y1<sps.pic_height_in_luma_samples)
      read_coding_quadtree(tctx,x1,y1, log2CbSize-1, ctDepth+1);
  }
  else {
    // set ctDepth of this CU

    img->set_ctDepth(x0,y0, log2CbSize, ctDepth);

    read_coding_unit(tctx, x0,y0, log2CbSize, ctDepth);
  }

  logtrace(LogSlice,"-\n");
}