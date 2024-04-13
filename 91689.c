WORD32 ihevcd_set_flush_mode(iv_obj_t *ps_codec_obj,
 void *pv_api_ip,
 void *pv_api_op)
{

 codec_t *ps_codec;
 ivd_ctl_flush_op_t *ps_ctl_op = (ivd_ctl_flush_op_t *)pv_api_op;
    UNUSED(pv_api_ip);
    ps_codec = (codec_t *)(ps_codec_obj->pv_codec_handle);

 /* Signal flush frame control call */
    ps_codec->i4_flush_mode = 1;

    ps_ctl_op->u4_error_code = 0;

 /* Set pic count to zero, so that decoder starts buffering again */
 /* once it comes out of flush mode */
    ps_codec->u4_pic_cnt = 0;
    ps_codec->u4_disp_cnt = 0;
 return IV_SUCCESS;


}
