de265_error slice_segment_header::write(error_queue* errqueue, CABAC_encoder& out,
                                        const seq_parameter_set* sps,
                                        const pic_parameter_set* pps,
                                        uint8_t nal_unit_type)
{
  out.write_bit(first_slice_segment_in_pic_flag);

  if (isRapPic(nal_unit_type)) { // TODO: is this still correct ? Should we drop RapPicFlag ?
    out.write_bit(no_output_of_prior_pics_flag);
  }

  if (slice_pic_parameter_set_id > DE265_MAX_PPS_SETS) {
    errqueue->add_warning(DE265_WARNING_NONEXISTING_PPS_REFERENCED, false);
    return DE265_OK;
  }
  out.write_uvlc(slice_pic_parameter_set_id);

  if (!first_slice_segment_in_pic_flag) {
    if (pps->dependent_slice_segments_enabled_flag) {
      out.write_bit(dependent_slice_segment_flag);
    }

    out.write_bits(slice_segment_address, ceil_log2(sps->PicSizeInCtbsY));

    if (dependent_slice_segment_flag) {
      if (slice_segment_address == 0) {
        errqueue->add_warning(DE265_WARNING_DEPENDENT_SLICE_WITH_ADDRESS_ZERO, false);
        return DE265_OK;
      }
    }
  }

  if (slice_segment_address < 0 ||
      slice_segment_address > sps->PicSizeInCtbsY) {
    errqueue->add_warning(DE265_WARNING_SLICE_SEGMENT_ADDRESS_INVALID, false);
    return DE265_ERROR_CODED_PARAMETER_OUT_OF_RANGE;
  }



  if (!dependent_slice_segment_flag) {
    for (int i=0; i<pps->num_extra_slice_header_bits; i++) {
      //slice_reserved_undetermined_flag[i]
      out.skip_bits(1);
    }

    if (slice_type > 2) {
      errqueue->add_warning(DE265_WARNING_SLICEHEADER_INVALID, false);
      return DE265_OK;
    }
    out.write_uvlc(slice_type);

    if (pps->output_flag_present_flag) {
      out.write_bit(pic_output_flag);
    }

    if (sps->separate_colour_plane_flag == 1) {
      out.write_bits(colour_plane_id,2);
    }


    int NumLtPics = 0;

    if (nal_unit_type != NAL_UNIT_IDR_W_RADL &&
        nal_unit_type != NAL_UNIT_IDR_N_LP) {
      out.write_bits(slice_pic_order_cnt_lsb, sps->log2_max_pic_order_cnt_lsb);
      out.write_bit(short_term_ref_pic_set_sps_flag);

      if (!short_term_ref_pic_set_sps_flag) {
        /* TODO
        read_short_term_ref_pic_set(ctx, sps,
                                    br, &slice_ref_pic_set,
                                    sps->num_short_term_ref_pic_sets,
                                    sps->ref_pic_sets,
                                    true);
        */
        //CurrRpsIdx = sps->num_short_term_ref_pic_sets;
        //CurrRps    = slice_ref_pic_set;
      }
      else {
        int nBits = ceil_log2(sps->num_short_term_ref_pic_sets());
        if (nBits>0) out.write_bits(short_term_ref_pic_set_idx,nBits);
        else         { assert(short_term_ref_pic_set_idx==0); }

        if (short_term_ref_pic_set_idx > sps->num_short_term_ref_pic_sets()) {
          errqueue->add_warning(DE265_WARNING_SHORT_TERM_REF_PIC_SET_OUT_OF_RANGE, false);
          return DE265_ERROR_CODED_PARAMETER_OUT_OF_RANGE;
        }

        //CurrRpsIdx = short_term_ref_pic_set_idx;
        //CurrRps    = sps->ref_pic_sets[CurrRpsIdx];
      }


      // --- long-term MC ---

      if (sps->long_term_ref_pics_present_flag) {
        if (sps->num_long_term_ref_pics_sps > 0) {
          out.write_uvlc(num_long_term_sps);
        }
        else {
          assert(num_long_term_sps == 0);
        }

        out.write_uvlc(num_long_term_pics);


        // check maximum number of reference frames

        if (num_long_term_sps +
            num_long_term_pics +
            CurrRps.NumNegativePics +
            CurrRps.NumPositivePics
            > sps->sps_max_dec_pic_buffering[sps->sps_max_sub_layers-1])
          {
            errqueue->add_warning(DE265_WARNING_MAX_NUM_REF_PICS_EXCEEDED, false);
            return DE265_OK;
          }

        for (int i=0; i<num_long_term_sps + num_long_term_pics; i++) {
          if (i < num_long_term_sps) {
            int nBits = ceil_log2(sps->num_long_term_ref_pics_sps);
            out.write_bits(lt_idx_sps[i], nBits);

            // check that the referenced lt-reference really exists

            if (lt_idx_sps[i] >= sps->num_long_term_ref_pics_sps) {
              errqueue->add_warning(DE265_NON_EXISTING_LT_REFERENCE_CANDIDATE_IN_SLICE_HEADER, false);
              return DE265_OK;
            }

            //ctx->PocLsbLt[i] = sps->lt_ref_pic_poc_lsb_sps[ lt_idx_sps[i] ];
            //ctx->UsedByCurrPicLt[i] = sps->used_by_curr_pic_lt_sps_flag[ lt_idx_sps[i] ];
          }
          else {
            int nBits = sps->log2_max_pic_order_cnt_lsb;
            out.write_bits(poc_lsb_lt[i], nBits);
            out.write_bit(used_by_curr_pic_lt_flag[i]);

            //ctx->PocLsbLt[i] = poc_lsb_lt[i];
            //ctx->UsedByCurrPicLt[i] = used_by_curr_pic_lt_flag[i];
          }

          //if (ctx->UsedByCurrPicLt[i]) {
          //NumLtPics++;
          //}

          out.write_bit(delta_poc_msb_present_flag[i]);
          if (delta_poc_msb_present_flag[i]) {
            out.write_uvlc(delta_poc_msb_cycle_lt[i]);
          }
          else {
            assert(delta_poc_msb_cycle_lt[i] == 0);
          }

          /*
          if (i==0 || i==num_long_term_sps) {
            ctx->DeltaPocMsbCycleLt[i] = delta_poc_msb_cycle_lt[i];
          }
          else {
            ctx->DeltaPocMsbCycleLt[i] = (delta_poc_msb_cycle_lt[i] +
                                          ctx->DeltaPocMsbCycleLt[i-1]);
          }
          */
        }
      }
      else {
        assert(num_long_term_sps == 0);
        assert(num_long_term_pics== 0);
      }

      if (sps->sps_temporal_mvp_enabled_flag) {
        out.write_bit(slice_temporal_mvp_enabled_flag);
      }
      else {
        assert(slice_temporal_mvp_enabled_flag == 0);
      }
    }
    else {
      assert(slice_pic_order_cnt_lsb == 0);
      assert(num_long_term_sps == 0);
      assert(num_long_term_pics== 0);
    }


    // --- SAO ---

    if (sps->sample_adaptive_offset_enabled_flag) {
      out.write_bit(slice_sao_luma_flag);
      out.write_bit(slice_sao_chroma_flag);
    }
    else {
      assert(slice_sao_luma_flag  == 0);
      assert(slice_sao_chroma_flag== 0);
    }

    if (slice_type == SLICE_TYPE_P  ||
        slice_type == SLICE_TYPE_B) {
      out.write_bit(num_ref_idx_active_override_flag);

      if (num_ref_idx_active_override_flag) {
        out.write_uvlc(num_ref_idx_l0_active);
        num_ref_idx_l0_active++;;

        if (slice_type == SLICE_TYPE_B) {
          out.write_uvlc(num_ref_idx_l1_active);
          num_ref_idx_l1_active++;
        }
      }
      else {
        assert(num_ref_idx_l0_active == pps->num_ref_idx_l0_default_active);
        assert(num_ref_idx_l1_active == pps->num_ref_idx_l1_default_active);
      }

      NumPocTotalCurr = CurrRps.NumPocTotalCurr_shortterm_only + NumLtPics;

      if (pps->lists_modification_present_flag && NumPocTotalCurr > 1) {

        int nBits = ceil_log2(NumPocTotalCurr);

        out.write_bit(ref_pic_list_modification_flag_l0);
        if (ref_pic_list_modification_flag_l0) {
          for (int i=0;i<num_ref_idx_l0_active;i++) {
            out.write_bits(list_entry_l0[i], nBits);
          }
        }

        if (slice_type == SLICE_TYPE_B) {
          out.write_bit(ref_pic_list_modification_flag_l1);
          if (ref_pic_list_modification_flag_l1) {
            for (int i=0;i<num_ref_idx_l1_active;i++) {
              out.write_bits(list_entry_l1[i], nBits);
            }
          }
        }
        else {
          assert(ref_pic_list_modification_flag_l1 == 0);
        }
      }
      else {
        assert(ref_pic_list_modification_flag_l0 == 0);
        assert(ref_pic_list_modification_flag_l1 == 0);
      }

      if (slice_type == SLICE_TYPE_B) {
        out.write_bit(mvd_l1_zero_flag);
      }

      if (pps->cabac_init_present_flag) {
        out.write_bit(cabac_init_flag);
      }
      else {
        assert(cabac_init_flag == 0);
      }

      if (slice_temporal_mvp_enabled_flag) {
        if (slice_type == SLICE_TYPE_B)
          out.write_bit(collocated_from_l0_flag);
        else
          { assert(collocated_from_l0_flag == 1); }

        if (( collocated_from_l0_flag && num_ref_idx_l0_active > 1) ||
            (!collocated_from_l0_flag && num_ref_idx_l1_active > 1)) {
          out.write_uvlc(collocated_ref_idx);
        }
        else {
          assert(collocated_ref_idx == 0);
        }
      }

      if ((pps->weighted_pred_flag   && slice_type == SLICE_TYPE_P) ||
          (pps->weighted_bipred_flag && slice_type == SLICE_TYPE_B)) {

        assert(0);
        /* TODO
        if (!read_pred_weight_table(br,this,ctx))
          {
	    ctx->add_warning(DE265_ERROR_CODED_PARAMETER_OUT_OF_RANGE, false);
	    return DE265_ERROR_CODED_PARAMETER_OUT_OF_RANGE;
          }
        */
      }

      out.write_uvlc(five_minus_max_num_merge_cand);
      //MaxNumMergeCand = 5-five_minus_max_num_merge_cand;
    }

    out.write_svlc(slice_qp_delta);

    if (pps->pps_slice_chroma_qp_offsets_present_flag) {
      out.write_svlc(slice_cb_qp_offset);
      out.write_svlc(slice_cr_qp_offset);
    }
    else {
      assert(slice_cb_qp_offset == 0);
      assert(slice_cr_qp_offset == 0);
    }

    if (pps->deblocking_filter_override_enabled_flag) {
      out.write_bit(deblocking_filter_override_flag);
    }
    else {
      assert(deblocking_filter_override_flag == 0);
    }

    //slice_beta_offset = pps->beta_offset;
    //slice_tc_offset   = pps->tc_offset;

    if (deblocking_filter_override_flag) {
      out.write_bit(slice_deblocking_filter_disabled_flag);
      if (!slice_deblocking_filter_disabled_flag) {
        out.write_svlc(slice_beta_offset/2);
        out.write_svlc(slice_tc_offset  /2);
      }
    }
    else {
      assert(slice_deblocking_filter_disabled_flag == pps->pic_disable_deblocking_filter_flag);
    }

    if (pps->pps_loop_filter_across_slices_enabled_flag  &&
        (slice_sao_luma_flag || slice_sao_chroma_flag ||
         !slice_deblocking_filter_disabled_flag )) {
      out.write_bit(slice_loop_filter_across_slices_enabled_flag);
    }
    else {
      assert(slice_loop_filter_across_slices_enabled_flag ==
             pps->pps_loop_filter_across_slices_enabled_flag);
    }
  }

  if (pps->tiles_enabled_flag || pps->entropy_coding_sync_enabled_flag ) {
    out.write_uvlc(num_entry_point_offsets);

    if (num_entry_point_offsets > 0) {
      out.write_uvlc(offset_len-1);

      for (int i=0; i<num_entry_point_offsets; i++) {
        {
          int prev=0;
          if (i>0) prev = entry_point_offset[i-1];
          out.write_bits(entry_point_offset[i]-prev-1, offset_len);
        }
      }
    }
  }
  else {
    assert(num_entry_point_offsets == 0);
  }

  if (pps->slice_segment_header_extension_present_flag) {
    out.write_uvlc(slice_segment_header_extension_length);
    if (slice_segment_header_extension_length > 1000) {  // TODO: safety check against too large values
      errqueue->add_warning(DE265_WARNING_SLICEHEADER_INVALID, false);
      return DE265_ERROR_CODED_PARAMETER_OUT_OF_RANGE;
    }

    for (int i=0; i<slice_segment_header_extension_length; i++) {
      //slice_segment_header_extension_data_byte[i]
      out.skip_bits(8);
    }
  }

  return DE265_OK;
}