int check_CTB_available(const de265_image* img,
                        int xC,int yC, int xN,int yN)
{
  // check whether neighbor is outside of frame

  if (xN < 0 || yN < 0) { return 0; }
  if (xN >= img->get_sps().pic_width_in_luma_samples)  { return 0; }
  if (yN >= img->get_sps().pic_height_in_luma_samples) { return 0; }


  int current_ctbAddrRS  = luma_pos_to_ctbAddrRS(&img->get_sps(), xC,yC);
  int neighbor_ctbAddrRS = luma_pos_to_ctbAddrRS(&img->get_sps(), xN,yN);

  // TODO: check if this is correct (6.4.1)

  if (img->get_SliceAddrRS_atCtbRS(current_ctbAddrRS) !=
      img->get_SliceAddrRS_atCtbRS(neighbor_ctbAddrRS)) {
    return 0;
  }

  // check if both CTBs are in the same tile.

  if (img->get_pps().TileIdRS[current_ctbAddrRS] !=
      img->get_pps().TileIdRS[neighbor_ctbAddrRS]) {
    return 0;
  }

  return 1;
}