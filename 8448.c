void slice_segment_header::set_defaults()
{
  slice_index = 0;

  first_slice_segment_in_pic_flag = 1;
  no_output_of_prior_pics_flag = 0;
  slice_pic_parameter_set_id = 0;
  dependent_slice_segment_flag = 0;
  slice_segment_address = 0;

  slice_type = SLICE_TYPE_I;
  pic_output_flag = 1;
  colour_plane_id = 0;
  slice_pic_order_cnt_lsb = 0;
  short_term_ref_pic_set_sps_flag = 1;
  // ref_pic_set slice_ref_pic_set;

  short_term_ref_pic_set_idx = 0;
  num_long_term_sps = 0;
  num_long_term_pics = 0;

  //uint8_t lt_idx_sps[MAX_NUM_REF_PICS];
  //int     poc_lsb_lt[MAX_NUM_REF_PICS];
  //char    used_by_curr_pic_lt_flag[MAX_NUM_REF_PICS];

  //char delta_poc_msb_present_flag[MAX_NUM_REF_PICS];
  //int delta_poc_msb_cycle_lt[MAX_NUM_REF_PICS];

  slice_temporal_mvp_enabled_flag = 0;
  slice_sao_luma_flag = 0;
  slice_sao_chroma_flag = 0;

  num_ref_idx_active_override_flag = 0;
  num_ref_idx_l0_active=1; // [1;16]
  num_ref_idx_l1_active=1; // [1;16]

  ref_pic_list_modification_flag_l0 = 0;
  ref_pic_list_modification_flag_l1 = 0;
  //uint8_t list_entry_l0[16];
  //uint8_t list_entry_l1[16];

  mvd_l1_zero_flag = 0;
  cabac_init_flag = 0;
  collocated_from_l0_flag = 0;
  collocated_ref_idx = 0;

  // --- pred_weight_table ---

  luma_log2_weight_denom=0; // [0;7]
  ChromaLog2WeightDenom=0;  // [0;7]

  // first index is L0/L1
  /*
  uint8_t luma_weight_flag[2][16];   // bool
  uint8_t chroma_weight_flag[2][16]; // bool
  int16_t LumaWeight[2][16];
  int8_t  luma_offset[2][16];
  int16_t ChromaWeight[2][16][2];
  int8_t  ChromaOffset[2][16][2];
  */


  five_minus_max_num_merge_cand = 0;
  slice_qp_delta = 0;

  slice_cb_qp_offset = 0;
  slice_cr_qp_offset = 0;

  cu_chroma_qp_offset_enabled_flag = 0;

  deblocking_filter_override_flag = 0;
  slice_deblocking_filter_disabled_flag = 0;
  slice_beta_offset=0; // = pps->beta_offset if undefined
  slice_tc_offset=0;   // = pps->tc_offset if undefined

  slice_loop_filter_across_slices_enabled_flag = 0;

  num_entry_point_offsets = 0;
  //int  offset_len;
  //std::vector<int> entry_point_offset;

  slice_segment_header_extension_length = 0;

  SliceAddrRS = slice_segment_address;
}