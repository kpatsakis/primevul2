UWORD32 ih264d_map_error(UWORD32 i4_err_status)
{
    UWORD32 temp = 0;

 switch(i4_err_status)
 {
 case ERROR_MEM_ALLOC_ISRAM_T:
 case ERROR_MEM_ALLOC_SDRAM_T:
 case ERROR_BUF_MGR:
 case ERROR_MB_GROUP_ASSGN_T:
 case ERROR_FRAME_LIMIT_OVER:
 case ERROR_ACTUAL_RESOLUTION_GREATER_THAN_INIT:
 case ERROR_PROFILE_NOT_SUPPORTED:
 case ERROR_INIT_NOT_DONE:
 case IVD_MEM_ALLOC_FAILED:
            temp = 1 << IVD_FATALERROR;
            H264_DEC_DEBUG_PRINT("\nFatal Error\n");
 break;

 case ERROR_DBP_MANAGER_T:
 case ERROR_GAPS_IN_FRM_NUM:
 case ERROR_UNKNOWN_NAL:
 case ERROR_INV_MB_SLC_GRP_T:
 case ERROR_MULTIPLE_SLC_GRP_T:
 case ERROR_UNKNOWN_LEVEL:
 case ERROR_UNAVAIL_PICBUF_T:
 case ERROR_UNAVAIL_MVBUF_T:
 case ERROR_UNAVAIL_DISPBUF_T:
 case ERROR_NUM_REF:
 case ERROR_REFIDX_ORDER_T:
 case ERROR_PIC0_NOT_FOUND_T:
 case ERROR_MB_TYPE:
 case ERROR_SUB_MB_TYPE:
 case ERROR_CBP:
 case ERROR_REF_IDX:
 case ERROR_NUM_MV:
 case ERROR_CHROMA_PRED_MODE:
 case ERROR_INTRAPRED:
 case ERROR_NEXT_MB_ADDRESS_T:
 case ERROR_MB_ADDRESS_T:
 case ERROR_PIC1_NOT_FOUND_T:
 case ERROR_CAVLC_NUM_COEFF_T:
 case ERROR_CAVLC_SCAN_POS_T:
 case ERROR_PRED_WEIGHT_TABLE_T:
 case ERROR_CORRUPTED_SLICE:
            temp = 1 << IVD_CORRUPTEDDATA;
 break;

 case ERROR_NOT_SUPP_RESOLUTION:
 case ERROR_FEATURE_UNAVAIL:
 case ERROR_ACTUAL_LEVEL_GREATER_THAN_INIT:
            temp = 1 << IVD_UNSUPPORTEDINPUT;
 break;

 case ERROR_INVALID_PIC_PARAM:
 case ERROR_INVALID_SEQ_PARAM:
 case ERROR_EGC_EXCEED_32_1_T:
 case ERROR_EGC_EXCEED_32_2_T:
 case ERROR_INV_RANGE_TEV_T:
 case ERROR_INV_SLC_TYPE_T:
 case ERROR_INV_POC_TYPE_T:
 case ERROR_INV_RANGE_QP_T:
 case ERROR_INV_SPS_PPS_T:
 case ERROR_INV_SLICE_HDR_T:
            temp = 1 << IVD_CORRUPTEDHEADER;
 break;

 case ERROR_EOB_FLUSHBITS_T:
 case ERROR_EOB_GETBITS_T:
 case ERROR_EOB_GETBIT_T:
 case ERROR_EOB_BYPASS_T:
 case ERROR_EOB_DECISION_T:
 case ERROR_EOB_TERMINATE_T:
 case ERROR_EOB_READCOEFF4X4CAB_T:
            temp = 1 << IVD_INSUFFICIENTDATA;
 break;
 case ERROR_DYNAMIC_RESOLUTION_NOT_SUPPORTED:
 case ERROR_DISP_WIDTH_RESET_TO_PIC_WIDTH:
            temp = 1 << IVD_UNSUPPORTEDPARAM | 1 << IVD_FATALERROR;
 break;

 case ERROR_DANGLING_FIELD_IN_PIC:
            temp = 1 << IVD_APPLIEDCONCEALMENT;
 break;

 }

 return temp;

}
