void slice_segment_header::compute_derived_values(const pic_parameter_set* pps)
{
  // --- init variables ---

  SliceQPY = pps->pic_init_qp + slice_qp_delta;

  switch (slice_type)
    {
    case SLICE_TYPE_I: initType = 0; break;
    case SLICE_TYPE_P: initType = cabac_init_flag + 1; break;
    case SLICE_TYPE_B: initType = 2 - cabac_init_flag; break;
    }

  MaxNumMergeCand = 5-five_minus_max_num_merge_cand;
}