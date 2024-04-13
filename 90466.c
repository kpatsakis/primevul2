int aacDecoder_drcMarkPayload (
    HANDLE_AAC_DRC self,
    HANDLE_FDK_BITSTREAM bs,
    AACDEC_DRC_PAYLOAD_TYPE type )
{
  UINT bsStartPos;
 int  i, numBands = 1, bitCnt = 0;

 if (self == NULL) {
 return 0;
 }

  bsStartPos = FDKgetValidBits(bs);

 switch (type) {
 case MPEG_DRC_EXT_DATA:
 {
      bitCnt = 4;

 if (FDKreadBits(bs,1)) { /* pce_tag_present */
 FDKreadBits(bs,8); /* pce_instance_tag + drc_tag_reserved_bits */
        bitCnt+=8;
 }

 if (FDKreadBits(bs,1)) { /* excluded_chns_present */
 FDKreadBits(bs,7); /* exclude mask [0..7] */
        bitCnt+=8;
 while (FDKreadBits(bs,1)) { /* additional_excluded_chns */
 FDKreadBits(bs,7); /* exclude mask [x..y] */
          bitCnt+=8;
 }
 }

 if (FDKreadBits(bs,1)) { /* drc_bands_present */
        numBands += FDKreadBits(bs, 4); /* drc_band_incr */
 FDKreadBits(bs,4); /* reserved */
        bitCnt+=8;
 for (i = 0; i < numBands; i++) {
 FDKreadBits(bs,8); /* drc_band_top[i] */
          bitCnt+=8;
 }
 }

 if (FDKreadBits(bs,1)) { /* prog_ref_level_present */
 FDKreadBits(bs,8); /* prog_ref_level + prog_ref_level_reserved_bits */
        bitCnt+=8;
 }

 for (i = 0; i < numBands; i++) {
 FDKreadBits(bs,8); /* dyn_rng_sgn[i] + dyn_rng_ctl[i] */
        bitCnt+=8;
 }

 if ( (self->numPayloads < MAX_DRC_THREADS)
 && ((INT)FDKgetValidBits(bs) >= 0) )
 {
        self->drcPayloadPosition[self->numPayloads++] = bsStartPos;
 }
 }
 break;

 case DVB_DRC_ANC_DATA:
      bitCnt += 8;
 /* check sync word */
 if (FDKreadBits(bs, 8) == DVB_ANC_DATA_SYNC_BYTE)
 {
 int dmxLevelsPresent, compressionPresent;
 int coarseGrainTcPresent, fineGrainTcPresent;

 /* bs_info field */ 
 FDKreadBits(bs, 8); /* mpeg_audio_type, dolby_surround_mode, presentation_mode */
        bitCnt+=8;

 /* Evaluate ancillary_data_status */
 FDKreadBits(bs, 3); /* reserved, set to 0 */
        dmxLevelsPresent = FDKreadBits(bs, 1); /* downmixing_levels_MPEG4_status */
 FDKreadBits(bs, 1); /* reserved, set to 0 */
        compressionPresent   = FDKreadBits(bs, 1); /* audio_coding_mode_and_compression status */
        coarseGrainTcPresent = FDKreadBits(bs, 1); /* coarse_grain_timecode_status */
        fineGrainTcPresent   = FDKreadBits(bs, 1); /* fine_grain_timecode_status */
        bitCnt+=8;

 /* MPEG4 downmixing levels */
 if (dmxLevelsPresent) {
 FDKreadBits(bs, 8); /* downmixing_levels_MPEG4 */
          bitCnt+=8;
 }
 /* audio coding mode and compression status */
 if (compressionPresent) {
 FDKreadBits(bs, 16); /* audio_coding_mode, Compression_value */
          bitCnt+=16;
 }
 /* coarse grain timecode */
 if (coarseGrainTcPresent) {
 FDKreadBits(bs, 16); /* coarse_grain_timecode */
          bitCnt+=16;
 }
 /* fine grain timecode */
 if (fineGrainTcPresent) {
 FDKreadBits(bs, 16); /* fine_grain_timecode */
          bitCnt+=16;
 }
 if ( !self->dvbAncDataAvailable
 && ((INT)FDKgetValidBits(bs) >= 0) )
 {
          self->dvbAncDataPosition  = bsStartPos;
          self->dvbAncDataAvailable = 1;
 }
 }
 break;

 default:
 break;
 }

 return (bitCnt);
}
