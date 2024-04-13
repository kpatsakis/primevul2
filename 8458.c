void slice_segment_header::reset()
{
  pps = NULL;

  slice_index = 0;

  first_slice_segment_in_pic_flag = 0;
  no_output_of_prior_pics_flag = 0;
  slice_pic_parameter_set_id = 0;
  dependent_slice_segment_flag = 0;
  slice_segment_address = 0;

  slice_type = 0;
  pic_output_flag = 0;
  colour_plane_id = 0;
  slice_pic_order_cnt_lsb = 0;
  short_term_ref_pic_set_sps_flag = 0;
  slice_ref_pic_set.reset();

  short_term_ref_pic_set_idx = 0;
  num_long_term_sps = 0;
  num_long_term_pics= 0;

  for (int i=0;i<MAX_NUM_REF_PICS;i++) {
    lt_idx_sps[i] = 0;
    poc_lsb_lt[i] = 0;
    used_by_curr_pic_lt_flag[i] = 0;
    delta_poc_msb_present_flag[i] = 0;
    delta_poc_msb_cycle_lt[i] = 0;
  }

  slice_temporal_mvp_enabled_flag = 0;
  slice_sao_luma_flag = 0;
  slice_sao_chroma_flag = 0;

  num_ref_idx_active_override_flag = 0;
  num_ref_idx_l0_active = 0;
  num_ref_idx_l1_active = 0;

  ref_pic_list_modification_flag_l0 = 0;
  ref_pic_list_modification_flag_l1 = 0;
  for (int i=0;i<16;i++) {
    list_entry_l0[i] = 0;
    list_entry_l1[i] = 0;
  }

  mvd_l1_zero_flag = 0;
  cabac_init_flag  = 0;
  collocated_from_l0_flag = 0;
  collocated_ref_idx = 0;

  luma_log2_weight_denom = 0;
  ChromaLog2WeightDenom  = 0;

  for (int i=0;i<2;i++)
    for (int j=0;j<16;j++) {
      luma_weight_flag[i][j] = 0;
      chroma_weight_flag[i][j] = 0;
      LumaWeight[i][j] = 0;
      luma_offset[i][j] = 0;
      ChromaWeight[i][j][0] = ChromaWeight[i][j][1] = 0;
      ChromaOffset[i][j][0] = ChromaOffset[i][j][1] = 0;
    }

  five_minus_max_num_merge_cand = 0;
  slice_qp_delta = 0;

  slice_cb_qp_offset = 0;
  slice_cr_qp_offset = 0;

  cu_chroma_qp_offset_enabled_flag = 0;

  deblocking_filter_override_flag = 0;
  slice_deblocking_filter_disabled_flag = 0;
  slice_beta_offset = 0;
  slice_tc_offset = 0;

  slice_loop_filter_across_slices_enabled_flag = 0;

  num_entry_point_offsets = 0;
  offset_len = 0;
  entry_point_offset.clear();

  slice_segment_header_extension_length = 0;

  SliceAddrRS = 0;
  SliceQPY = 0;

  initType = 0;

  MaxNumMergeCand = 0;
  CurrRpsIdx = 0;
  CurrRps.reset();
  NumPocTotalCurr = 0;

  for (int i=0;i<2;i++)
    for (int j=0;j<MAX_NUM_REF_PICS;j++) {
      RefPicList[i][j] = 0;
      RefPicList_POC[i][j] = 0;
      RefPicList_PicState[i][j] = 0;
      LongTermRefPic[i][j] = 0;
    }

  //context_model ctx_model_storage[CONTEXT_MODEL_TABLE_LENGTH];

  RemoveReferencesList.clear();

  ctx_model_storage_defined = false;
}