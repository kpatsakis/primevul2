status_t HevcParameterSets::parseSps(const uint8_t* data, size_t size) {
 NALBitReader reader(data, size);
    reader.skipBits(4);
 uint8_t maxSubLayersMinus1 = reader.getBitsWithFallback(3, 0);
    reader.skipBits(1);
    reader.skipBits(96);
 if (maxSubLayersMinus1 > 0) {
 bool subLayerProfilePresentFlag[8];
 bool subLayerLevelPresentFlag[8];
 for (int i = 0; i < maxSubLayersMinus1; ++i) {
            subLayerProfilePresentFlag[i] = reader.getBitsWithFallback(1, 0);
            subLayerLevelPresentFlag[i] = reader.getBitsWithFallback(1, 0);
 }
        reader.skipBits(2 * (8 - maxSubLayersMinus1));
 for (int i = 0; i < maxSubLayersMinus1; ++i) {
 if (subLayerProfilePresentFlag[i]) {
                reader.skipBits(88);
 }
 if (subLayerLevelPresentFlag[i]) {
                reader.skipBits(8);
 }
 }
 }
    skipUE(&reader);
 uint8_t chromaFormatIdc = parseUEWithFallback(&reader, 0);
    mParams.add(kChromaFormatIdc, chromaFormatIdc);
 if (chromaFormatIdc == 3) {
        reader.skipBits(1);
 }
    skipUE(&reader);
    skipUE(&reader);
 if (reader.getBitsWithFallback(1, 0) /* i.e. conformance_window_flag */) {
        skipUE(&reader);
        skipUE(&reader);
        skipUE(&reader);
        skipUE(&reader);
 }
    mParams.add(kBitDepthLumaMinus8, parseUEWithFallback(&reader, 0));
    mParams.add(kBitDepthChromaMinus8, parseUEWithFallback(&reader, 0));

 size_t log2MaxPicOrderCntLsb = parseUEWithFallback(&reader, 0) + (size_t)4;
 bool spsSubLayerOrderingInfoPresentFlag = reader.getBitsWithFallback(1, 0);
 for (uint32_t i = spsSubLayerOrderingInfoPresentFlag ? 0 : maxSubLayersMinus1;
            i <= maxSubLayersMinus1; ++i) {
        skipUE(&reader); // sps_max_dec_pic_buffering_minus1[i]
        skipUE(&reader); // sps_max_num_reorder_pics[i]
        skipUE(&reader); // sps_max_latency_increase_plus1[i]
 }

    skipUE(&reader); // log2_min_luma_coding_block_size_minus3
    skipUE(&reader); // log2_diff_max_min_luma_coding_block_size
    skipUE(&reader); // log2_min_luma_transform_block_size_minus2
    skipUE(&reader); // log2_diff_max_min_luma_transform_block_size
    skipUE(&reader); // max_transform_hierarchy_depth_inter
    skipUE(&reader); // max_transform_hierarchy_depth_intra
 if (reader.getBitsWithFallback(1, 0)) { // scaling_list_enabled_flag u(1)
 if (reader.getBitsWithFallback(1, 0)) { // sps_scaling_list_data_present_flag
 for (uint32_t sizeId = 0; sizeId < 4; ++sizeId) {
 for (uint32_t matrixId = 0; matrixId < 6; matrixId += (sizeId == 3) ? 3 : 1) {
 if (!reader.getBitsWithFallback(1, 1)) {
                        skipUE(&reader); // scaling_list_pred_matrix_id_delta[sizeId][matrixId]
 } else {
 uint32_t coefNum = std::min(64, (1 << (4 + (sizeId << 1))));
 if (sizeId > 1) {
                            skipSE(&reader); // scaling_list_dc_coef_minus8[sizeId − 2][matrixId]
 }
 for (uint32_t i = 0; i < coefNum; ++i) {
                            skipSE(&reader); // scaling_list_delta_coef
 }
 }
 }
 }
 }
 }
    reader.skipBits(1); // amp_enabled_flag
    reader.skipBits(1); // sample_adaptive_offset_enabled_flag u(1)
 if (reader.getBitsWithFallback(1, 0)) { // pcm_enabled_flag
        reader.skipBits(4); // pcm_sample_bit_depth_luma_minus1
        reader.skipBits(4); // pcm_sample_bit_depth_chroma_minus1 u(4)
        skipUE(&reader); // log2_min_pcm_luma_coding_block_size_minus3
        skipUE(&reader); // log2_diff_max_min_pcm_luma_coding_block_size
        reader.skipBits(1); // pcm_loop_filter_disabled_flag
 }
 uint32_t numShortTermRefPicSets = parseUEWithFallback(&reader, 0);
 uint32_t numPics = 0;
 for (uint32_t i = 0; i < numShortTermRefPicSets; ++i) {
 if (i != 0 && reader.getBitsWithFallback(1, 0)) { // inter_ref_pic_set_prediction_flag
            reader.skipBits(1); // delta_rps_sign
            skipUE(&reader); // abs_delta_rps_minus1
 uint32_t nextNumPics = 0;
 for (uint32_t j = 0; j <= numPics; ++j) {
 if (reader.getBitsWithFallback(1, 0) // used_by_curr_pic_flag[j]
 || reader.getBitsWithFallback(1, 0)) { // use_delta_flag[j]
 ++nextNumPics;
 }
 }
            numPics = nextNumPics;
 } else {
 uint32_t numNegativePics = parseUEWithFallback(&reader, 0);
 uint32_t numPositivePics = parseUEWithFallback(&reader, 0);
 if (numNegativePics > UINT32_MAX - numPositivePics) {
 return ERROR_MALFORMED;
 }
            numPics = numNegativePics + numPositivePics;
 for (uint32_t j = 0; j < numPics; ++j) {
                skipUE(&reader); // delta_poc_s0|1_minus1[i]
                reader.skipBits(1); // used_by_curr_pic_s0|1_flag[i]
 }
 }
 }
 if (reader.getBitsWithFallback(1, 0)) { // long_term_ref_pics_present_flag
 uint32_t numLongTermRefPicSps = parseUEWithFallback(&reader, 0);
 for (uint32_t i = 0; i < numLongTermRefPicSps; ++i) {
            reader.skipBits(log2MaxPicOrderCntLsb); // lt_ref_pic_poc_lsb_sps[i]
            reader.skipBits(1); // used_by_curr_pic_lt_sps_flag[i]
 }
 }
    reader.skipBits(1); // sps_temporal_mvp_enabled_flag
    reader.skipBits(1); // strong_intra_smoothing_enabled_flag
 if (reader.getBitsWithFallback(1, 0)) { // vui_parameters_present_flag
 if (reader.getBitsWithFallback(1, 0)) { // aspect_ratio_info_present_flag
 uint32_t aspectRatioIdc = reader.getBitsWithFallback(8, 0);
 if (aspectRatioIdc == 0xFF /* EXTENDED_SAR */) {
                reader.skipBits(16); // sar_width
                reader.skipBits(16); // sar_height
 }
 }
 if (reader.getBitsWithFallback(1, 0)) { // overscan_info_present_flag
            reader.skipBits(1); // overscan_appropriate_flag
 }
 if (reader.getBitsWithFallback(1, 0)) { // video_signal_type_present_flag
            reader.skipBits(3); // video_format
 uint32_t videoFullRangeFlag;
 if (reader.getBitsGraceful(1, &videoFullRangeFlag)) {
                mParams.add(kVideoFullRangeFlag, videoFullRangeFlag);
 }
 if (reader.getBitsWithFallback(1, 0)) { // colour_description_present_flag
                mInfo = (Info)(mInfo | kInfoHasColorDescription);
 uint32_t colourPrimaries, transferCharacteristics, matrixCoeffs;
 if (reader.getBitsGraceful(8, &colourPrimaries)) {
                    mParams.add(kColourPrimaries, colourPrimaries);
 }
 if (reader.getBitsGraceful(8, &transferCharacteristics)) {
                    mParams.add(kTransferCharacteristics, transferCharacteristics);
 if (transferCharacteristics == 16 /* ST 2084 */
 || transferCharacteristics == 18 /* ARIB STD-B67 HLG */) {
                        mInfo = (Info)(mInfo | kInfoIsHdr);
 }
 }
 if (reader.getBitsGraceful(8, &matrixCoeffs)) {
                    mParams.add(kMatrixCoeffs, matrixCoeffs);
 }
 }
 }
 }

 return reader.overRead() ? ERROR_MALFORMED : OK;
}
