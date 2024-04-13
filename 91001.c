static UWORD32 ihevcd_map_error(IHEVCD_ERROR_T e_error)
{
    UWORD32 error_code = 0;
    error_code = e_error;
 switch(error_code)
 {
 case IHEVCD_SUCCESS :
 break;
 case IHEVCD_INIT_NOT_DONE:
 case IHEVCD_LEVEL_UNSUPPORTED:
 case IHEVCD_NUM_REF_UNSUPPORTED:
 case IHEVCD_NUM_REORDER_UNSUPPORTED:
 case IHEVCD_NUM_EXTRA_DISP_UNSUPPORTED:
 case IHEVCD_INSUFFICIENT_MEM_MVBANK:
 case IHEVCD_INSUFFICIENT_MEM_PICBUF:
            error_code |= 1 << IVD_FATALERROR;
 break;
 case IHEVCD_INVALID_DISP_STRD:
 case IHEVCD_CXA_VERS_BUF_INSUFFICIENT:
 case IHEVCD_UNSUPPORTED_VPS_ID:
 case IHEVCD_UNSUPPORTED_SPS_ID:
 case IHEVCD_UNSUPPORTED_PPS_ID:
 case IHEVCD_UNSUPPORTED_CHROMA_FMT_IDC:
 case IHEVCD_UNSUPPORTED_BIT_DEPTH:
 case IHEVCD_BUF_MGR_ERROR:
 case IHEVCD_NO_FREE_MVBANK:
 case IHEVCD_NO_FREE_PICBUF:
 case IHEVCD_SLICE_IN_HEADER_MODE:
 case IHEVCD_END_OF_SEQUENCE:
 break;
 default:
 break;
 }
 return error_code;
}
