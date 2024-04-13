WORD32 ihevcd_decode(iv_obj_t *ps_codec_obj, void *pv_api_ip, void *pv_api_op)
{
    WORD32 ret = IV_SUCCESS;
 codec_t *ps_codec = (codec_t *)(ps_codec_obj->pv_codec_handle);
 ivd_video_decode_ip_t *ps_dec_ip;
 ivd_video_decode_op_t *ps_dec_op;

    WORD32 proc_idx = 0;
    WORD32 prev_proc_idx = 0;

 /* Initialize error code */
    ps_codec->i4_error_code = 0;

    ps_dec_ip = (ivd_video_decode_ip_t *)pv_api_ip;
    ps_dec_op = (ivd_video_decode_op_t *)pv_api_op;

 {
        UWORD32 u4_size = ps_dec_op->u4_size;
        memset(ps_dec_op, 0, sizeof(ivd_video_decode_op_t));
        ps_dec_op->u4_size = u4_size; //Restore size field
 }
 if(ps_codec->i4_init_done != 1)
 {
        ps_dec_op->u4_error_code |= 1 << IVD_FATALERROR;
        ps_dec_op->u4_error_code |= IHEVCD_INIT_NOT_DONE;
 return IV_FAIL;
 }

 if(ps_codec->u4_pic_cnt >= NUM_FRAMES_LIMIT)
 {
        ps_dec_op->u4_error_code |= 1 << IVD_FATALERROR;
        ps_dec_op->u4_error_code |= IHEVCD_NUM_FRAMES_LIMIT_REACHED;
 return IV_FAIL;
 }

 /* If reset flag is set, flush the existing buffers */
 if(ps_codec->i4_reset_flag)
 {
        ps_codec->i4_flush_mode = 1;
 }

 /*Data memory barries instruction,so that bitstream write by the application is complete*/
 /* In case the decoder is not in flush mode check for input buffer validity */
 if(0 == ps_codec->i4_flush_mode)
 {
 if(ps_dec_ip->pv_stream_buffer == NULL)
 {
            ps_dec_op->u4_error_code |= 1 << IVD_UNSUPPORTEDPARAM;
            ps_dec_op->u4_error_code |= IVD_DEC_FRM_BS_BUF_NULL;
 return IV_FAIL;
 }
 if(ps_dec_ip->u4_num_Bytes <= MIN_START_CODE_LEN)
 {
 if((WORD32)ps_dec_ip->u4_num_Bytes > 0)
                ps_dec_op->u4_num_bytes_consumed = ps_dec_ip->u4_num_Bytes;
 else
                ps_dec_op->u4_num_bytes_consumed = 0;

            ps_dec_op->u4_error_code |= 1 << IVD_UNSUPPORTEDPARAM;
            ps_dec_op->u4_error_code |= IVD_DEC_NUMBYTES_INV;
 return IV_FAIL;

 }
 }

#ifdef APPLY_CONCEALMENT
 {
        WORD32 num_mbs;

        num_mbs = (ps_codec->i4_wd * ps_codec->i4_ht + 255) >> 8;
 /* Reset MB Count at the beginning of every process call */
        ps_codec->mb_count = 0;
        memset(ps_codec->mb_map, 0, ((num_mbs + 7) >> 3));
 }
#endif

 if(0 == ps_codec->i4_share_disp_buf && ps_codec->i4_header_mode == 0)
 {
        UWORD32 i;
 if(ps_dec_ip->s_out_buffer.u4_num_bufs == 0)
 {
            ps_dec_op->u4_error_code |= 1 << IVD_UNSUPPORTEDPARAM;
            ps_dec_op->u4_error_code |= IVD_DISP_FRM_ZERO_OP_BUFS;
 return IV_FAIL;
 }

 for(i = 0; i < ps_dec_ip->s_out_buffer.u4_num_bufs; i++)
 {
 if(ps_dec_ip->s_out_buffer.pu1_bufs[i] == NULL)
 {
                ps_dec_op->u4_error_code |= 1 << IVD_UNSUPPORTEDPARAM;
                ps_dec_op->u4_error_code |= IVD_DISP_FRM_OP_BUF_NULL;
 return IV_FAIL;
 }

 if(ps_dec_ip->s_out_buffer.u4_min_out_buf_size[i] == 0)
 {
                ps_dec_op->u4_error_code |= 1 << IVD_UNSUPPORTEDPARAM;
                ps_dec_op->u4_error_code |= IVD_DISP_FRM_ZERO_OP_BUF_SIZE;
 return IV_FAIL;
 }
 }
 }

    ps_codec->ps_out_buffer = &ps_dec_ip->s_out_buffer;
    ps_codec->u4_ts = ps_dec_ip->u4_ts;
 if(ps_codec->i4_flush_mode)
 {

        ps_dec_op->u4_pic_wd = ps_codec->i4_disp_wd;
        ps_dec_op->u4_pic_ht = ps_codec->i4_disp_ht;

        ps_dec_op->u4_new_seq = 0;

        ps_codec->ps_disp_buf = (pic_buf_t *)ihevc_disp_mgr_get(
 (disp_mgr_t *)ps_codec->pv_disp_buf_mgr, &ps_codec->i4_disp_buf_id);
 /* In case of non-shared mode, then convert/copy the frame to output buffer */
 /* Only if the codec is in non-shared mode or in shared mode but needs 420P output */
 if((ps_codec->ps_disp_buf)
 && ((0 == ps_codec->i4_share_disp_buf)
 || (IV_YUV_420P
 == ps_codec->e_chroma_fmt)))
 {

 process_ctxt_t *ps_proc = &ps_codec->as_process[prev_proc_idx];
 if(0 == ps_proc->i4_init_done)
 {
                ihevcd_init_proc_ctxt(ps_proc, 0);
 }

 /* Set remaining number of rows to be processed */
            ret = ihevcd_fmt_conv(ps_codec, &ps_codec->as_process[prev_proc_idx],
                                  ps_dec_ip->s_out_buffer.pu1_bufs[0],
                                  ps_dec_ip->s_out_buffer.pu1_bufs[1],
                                  ps_dec_ip->s_out_buffer.pu1_bufs[2], 0,
                                  ps_codec->i4_disp_ht);

            ihevc_buf_mgr_release((buf_mgr_t *)ps_codec->pv_pic_buf_mgr,
                                  ps_codec->i4_disp_buf_id, BUF_MGR_DISP);
 }

        ihevcd_fill_outargs(ps_codec, ps_dec_ip, ps_dec_op);

 if(1 == ps_dec_op->u4_output_present)
 {
            WORD32 xpos = ps_codec->i4_disp_wd - 32 - LOGO_WD;
            WORD32 ypos = ps_codec->i4_disp_ht - 32 - LOGO_HT;

 if(ypos < 0)
                ypos = 0;

 if(xpos < 0)
                xpos = 0;

            INSERT_LOGO(ps_dec_ip->s_out_buffer.pu1_bufs[0],
                        ps_dec_ip->s_out_buffer.pu1_bufs[1],
                        ps_dec_ip->s_out_buffer.pu1_bufs[2], ps_codec->i4_disp_strd,
                        xpos,
                        ypos,
                        ps_codec->e_chroma_fmt,
                        ps_codec->i4_disp_wd,
                        ps_codec->i4_disp_ht);
 }


 if(NULL == ps_codec->ps_disp_buf)
 {
 /* If in flush mode and there are no more buffers to flush,
             * check for the reset flag and reset the decoder */
 if(ps_codec->i4_reset_flag)
 {
                ihevcd_init(ps_codec);
 }
 return (IV_FAIL);
 }

 return (IV_SUCCESS);

 }
 /* In case of shared mode, check if there is a free buffer for reconstruction */
 if((0 == ps_codec->i4_header_mode) && (1 == ps_codec->i4_share_disp_buf))
 {
        WORD32 buf_status;
        buf_status = 1;
 if(ps_codec->pv_pic_buf_mgr)
            buf_status = ihevc_buf_mgr_check_free((buf_mgr_t *)ps_codec->pv_pic_buf_mgr);

 /* If there is no free buffer, then return with an error code */
 if(0 == buf_status)
 {
            ps_dec_op->u4_error_code = IVD_DEC_REF_BUF_NULL;
            ps_dec_op->u4_error_code |= (1 << IVD_UNSUPPORTEDPARAM);
 return IV_FAIL;
 }
 }
    ps_codec->i4_bytes_remaining = ps_dec_ip->u4_num_Bytes;
    ps_codec->pu1_inp_bitsbuf = (UWORD8 *)ps_dec_ip->pv_stream_buffer;
    ps_codec->s_parse.i4_end_of_frame = 0;

    ps_codec->i4_pic_present = 0;
    ps_codec->i4_slice_error = 0;
    ps_codec->ps_disp_buf = NULL;

 if(ps_codec->i4_num_cores > 1)
 {
        ithread_set_affinity(0);
 }
 while(MIN_START_CODE_LEN < ps_codec->i4_bytes_remaining)
 {
        WORD32 nal_len;
        WORD32 nal_ofst;
        WORD32 bits_len;

 if(ps_codec->i4_slice_error)
 {
 slice_header_t *ps_slice_hdr_next = ps_codec->s_parse.ps_slice_hdr_base + (ps_codec->s_parse.i4_cur_slice_idx & (MAX_SLICE_HDR_CNT - 1));
            WORD32 next_slice_addr = ps_slice_hdr_next->i2_ctb_x +
                            ps_slice_hdr_next->i2_ctb_y * ps_codec->s_parse.ps_sps->i2_pic_wd_in_ctb;
 if(ps_codec->s_parse.i4_next_ctb_indx == next_slice_addr)
                ps_codec->i4_slice_error = 0;
 }

 if(ps_codec->pu1_bitsbuf_dynamic)
 {
            ps_codec->pu1_bitsbuf = ps_codec->pu1_bitsbuf_dynamic;
            ps_codec->u4_bitsbuf_size = ps_codec->u4_bitsbuf_size_dynamic;
 }
 else
 {
            ps_codec->pu1_bitsbuf = ps_codec->pu1_bitsbuf_static;
            ps_codec->u4_bitsbuf_size = ps_codec->u4_bitsbuf_size_static;
 }

        nal_ofst = ihevcd_nal_search_start_code(ps_codec->pu1_inp_bitsbuf,
                                                ps_codec->i4_bytes_remaining);

        ps_codec->i4_nal_ofst = nal_ofst;
 {
            WORD32 bytes_remaining = ps_codec->i4_bytes_remaining - nal_ofst;

            bytes_remaining = MIN((UWORD32)bytes_remaining, ps_codec->u4_bitsbuf_size);
            ihevcd_nal_remv_emuln_bytes(ps_codec->pu1_inp_bitsbuf + nal_ofst,
                                        ps_codec->pu1_bitsbuf,
                                        bytes_remaining,
 &nal_len, &bits_len);

 /* Decoder may read upto 8 extra bytes at the end of frame */
 /* These are not used, but still set them to zero to avoid uninitialized reads */
 if(bits_len < (WORD32)(ps_codec->u4_bitsbuf_size - 8))
 {
                memset(ps_codec->pu1_bitsbuf + bits_len, 0, 2 * sizeof(UWORD32));
 }
 }
 /* This may be used to update the offsets for tiles and entropy sync row offsets */
        ps_codec->i4_num_emln_bytes = nal_len - bits_len;
        ps_codec->i4_nal_len = nal_len;

        ihevcd_bits_init(&ps_codec->s_parse.s_bitstrm, ps_codec->pu1_bitsbuf,
                         bits_len);

        ret = ihevcd_nal_unit(ps_codec);

 /* If the frame is incomplete and
         * the bytes remaining is zero or a header is received,
         * complete the frame treating it to be in error */
 if(ps_codec->i4_pic_present &&
 (ps_codec->s_parse.i4_next_ctb_indx != ps_codec->s_parse.ps_sps->i4_pic_size_in_ctb))
 {
 if((ps_codec->i4_bytes_remaining - (nal_len + nal_ofst) <= MIN_START_CODE_LEN) ||
 (ps_codec->i4_header_in_slice_mode))
 {
 slice_header_t *ps_slice_hdr_next;

                ps_codec->s_parse.i4_cur_slice_idx--;
 if(ps_codec->s_parse.i4_cur_slice_idx < 0)
                    ps_codec->s_parse.i4_cur_slice_idx = 0;

                ps_slice_hdr_next = ps_codec->s_parse.ps_slice_hdr_base + ((ps_codec->s_parse.i4_cur_slice_idx + 1) & (MAX_SLICE_HDR_CNT - 1));
                ps_slice_hdr_next->i2_ctb_x = 0;
                ps_slice_hdr_next->i2_ctb_y = ps_codec->s_parse.ps_sps->i2_pic_ht_in_ctb;
                ps_codec->i4_slice_error = 1;
 continue;
 }
 }

 
         if(IHEVCD_IGNORE_SLICE == ret)
         {
            ps_codec->s_parse.i4_cur_slice_idx = MAX(0, (ps_codec->s_parse.i4_cur_slice_idx - 1));
             ps_codec->pu1_inp_bitsbuf += (nal_ofst + nal_len);
             ps_codec->i4_bytes_remaining -= (nal_ofst + nal_len);
 
 continue;
 }

 if((IVD_RES_CHANGED == ret) ||
 (IHEVCD_UNSUPPORTED_DIMENSIONS == ret))
 {
 break;
 }

 /* Update bytes remaining and bytes consumed and input bitstream pointer */
 /* Do not consume the NAL in the following cases */
 /* Slice header reached during header decode mode */
 /* TODO: Next picture's slice reached */
 if(ret != IHEVCD_SLICE_IN_HEADER_MODE)
 {
 if((0 == ps_codec->i4_slice_error) ||
 (ps_codec->i4_bytes_remaining - (nal_len + nal_ofst) <= MIN_START_CODE_LEN))
 {
                ps_codec->pu1_inp_bitsbuf += (nal_ofst + nal_len);
                ps_codec->i4_bytes_remaining -= (nal_ofst + nal_len);
 }
 if(ret != IHEVCD_SUCCESS)
 break;

 if(ps_codec->s_parse.i4_end_of_frame)
 break;
 }
 else
 {
            ret = IHEVCD_SUCCESS;
 break;
 }

 /* Allocate dynamic bitstream buffer once SPS is decoded */
 if((ps_codec->u4_allocate_dynamic_done == 0) && ps_codec->i4_sps_done)
 {
            WORD32 ret;
            ret = ihevcd_allocate_dynamic_bufs(ps_codec);
 if(ret != IV_SUCCESS)
 {
 /* Free any dynamic buffers that are allocated */
                ihevcd_free_dynamic_bufs(ps_codec);
                ps_codec->i4_error_code = IVD_MEM_ALLOC_FAILED;
                ps_dec_op->u4_error_code |= 1 << IVD_FATALERROR;
                ps_dec_op->u4_error_code |= IVD_MEM_ALLOC_FAILED;

 return IV_FAIL;
 }
 }

        BREAK_AFTER_SLICE_NAL();
 }

 if((ps_codec->u4_pic_cnt == 0) && (ret != IHEVCD_SUCCESS))
 {
        ps_codec->i4_error_code = ret;

        ihevcd_fill_outargs(ps_codec, ps_dec_ip, ps_dec_op);
 return IV_FAIL;
 }

 if(1 == ps_codec->i4_pic_present)
 {
        WORD32 i;
 sps_t *ps_sps = ps_codec->s_parse.ps_sps;
        ps_codec->i4_first_pic_done = 1;

 /*TODO temporary fix: end_of_frame is checked before adding format conversion to job queue         */
 if(ps_codec->i4_num_cores > 1 && ps_codec->s_parse.i4_end_of_frame)
 {

 /* Add job queue for format conversion / frame copy for each ctb row */
 /* Only if the codec is in non-shared mode or in shared mode but needs 420P output */
 process_ctxt_t *ps_proc;

 /* i4_num_cores - 1 contexts are currently being used by other threads */
            ps_proc = &ps_codec->as_process[ps_codec->i4_num_cores - 1];

 if((ps_codec->ps_disp_buf) &&
 ((0 == ps_codec->i4_share_disp_buf) || (IV_YUV_420P == ps_codec->e_chroma_fmt)))
 {
 /* If format conversion jobs were not issued in pic_init() add them here */
 if((0 == ps_codec->u4_enable_fmt_conv_ahead) ||
 (ps_codec->i4_disp_buf_id == ps_proc->i4_cur_pic_buf_id))
 for(i = 0; i < ps_sps->i2_pic_ht_in_ctb; i++)
 {
 proc_job_t s_job;
                        IHEVCD_ERROR_T ret;
                        s_job.i4_cmd = CMD_FMTCONV;
                        s_job.i2_ctb_cnt = 0;
                        s_job.i2_ctb_x = 0;
                        s_job.i2_ctb_y = i;
                        s_job.i2_slice_idx = 0;
                        s_job.i4_tu_coeff_data_ofst = 0;
                        ret = ihevcd_jobq_queue((jobq_t *)ps_codec->s_parse.pv_proc_jobq,
 &s_job, sizeof(proc_job_t), 1);
 if(ret != (IHEVCD_ERROR_T)IHEVCD_SUCCESS)
 return (WORD32)ret;
 }
 }
 /* Reached end of frame : Signal terminate */
 /* The terminate flag is checked only after all the jobs are dequeued */
            ret = ihevcd_jobq_terminate((jobq_t *)ps_codec->s_parse.pv_proc_jobq);

 while(1)
 {
                IHEVCD_ERROR_T ret;
 proc_job_t s_job;
 process_ctxt_t *ps_proc;

 /* i4_num_cores - 1 contexts are currently being used by other threads */
                ps_proc = &ps_codec->as_process[ps_codec->i4_num_cores - 1];

                ret = ihevcd_jobq_dequeue((jobq_t *)ps_proc->pv_proc_jobq, &s_job,
 sizeof(proc_job_t), 1);
 if((IHEVCD_ERROR_T)IHEVCD_SUCCESS != ret)
 break;

                ps_proc->i4_ctb_cnt = s_job.i2_ctb_cnt;
                ps_proc->i4_ctb_x = s_job.i2_ctb_x;
                ps_proc->i4_ctb_y = s_job.i2_ctb_y;
                ps_proc->i4_cur_slice_idx = s_job.i2_slice_idx;

 if(CMD_PROCESS == s_job.i4_cmd)
 {
                    ihevcd_init_proc_ctxt(ps_proc, s_job.i4_tu_coeff_data_ofst);

                    ihevcd_process(ps_proc);
 }
 else if(CMD_FMTCONV == s_job.i4_cmd)
 {
 sps_t *ps_sps = ps_codec->s_parse.ps_sps;
                    WORD32 num_rows = 1 << ps_sps->i1_log2_ctb_size;
 if(0 == ps_proc->i4_init_done)
 {
                        ihevcd_init_proc_ctxt(ps_proc, 0);
 }

                    num_rows = MIN(num_rows, (ps_codec->i4_disp_ht - (s_job.i2_ctb_y << ps_sps->i1_log2_ctb_size)));
 if(num_rows < 0)
                        num_rows = 0;

                    ihevcd_fmt_conv(ps_codec, ps_proc,
                                    ps_dec_ip->s_out_buffer.pu1_bufs[0],
                                    ps_dec_ip->s_out_buffer.pu1_bufs[1],
                                    ps_dec_ip->s_out_buffer.pu1_bufs[2],
                                    s_job.i2_ctb_y << ps_sps->i1_log2_ctb_size,
                                    num_rows);
 }
 }
 }
 /* In case of non-shared mode and while running in single core mode, then convert/copy the frame to output buffer */
 /* Only if the codec is in non-shared mode or in shared mode but needs 420P output */
 else if((ps_codec->ps_disp_buf) && ((0 == ps_codec->i4_share_disp_buf) ||
 (IV_YUV_420P == ps_codec->e_chroma_fmt)) &&
 (ps_codec->s_parse.i4_end_of_frame))
 {
 process_ctxt_t *ps_proc = &ps_codec->as_process[proc_idx];
 /* Set remaining number of rows to be processed */
            ps_codec->s_fmt_conv.i4_num_rows = ps_codec->i4_disp_ht
 - ps_codec->s_fmt_conv.i4_cur_row;
 if(0 == ps_proc->i4_init_done)
 {
                ihevcd_init_proc_ctxt(ps_proc, 0);
 }

 if(ps_codec->s_fmt_conv.i4_num_rows < 0)
                ps_codec->s_fmt_conv.i4_num_rows = 0;

            ret = ihevcd_fmt_conv(ps_codec, ps_proc,
                                  ps_dec_ip->s_out_buffer.pu1_bufs[0],
                                  ps_dec_ip->s_out_buffer.pu1_bufs[1],
                                  ps_dec_ip->s_out_buffer.pu1_bufs[2],
                                  ps_codec->s_fmt_conv.i4_cur_row,
                                  ps_codec->s_fmt_conv.i4_num_rows);
            ps_codec->s_fmt_conv.i4_cur_row += ps_codec->s_fmt_conv.i4_num_rows;

 }


        DEBUG_DUMP_MV_MAP(ps_codec);

 /* Mark MV Buf as needed for reference */
        ihevc_buf_mgr_set_status((buf_mgr_t *)ps_codec->pv_mv_buf_mgr,
                                 ps_codec->as_process[proc_idx].i4_cur_mv_bank_buf_id,
                                 BUF_MGR_REF);

 /* Mark pic buf as needed for reference */
        ihevc_buf_mgr_set_status((buf_mgr_t *)ps_codec->pv_pic_buf_mgr,
                                 ps_codec->as_process[proc_idx].i4_cur_pic_buf_id,
                                 BUF_MGR_REF);

 /* Mark pic buf as needed for display */
        ihevc_buf_mgr_set_status((buf_mgr_t *)ps_codec->pv_pic_buf_mgr,
                                 ps_codec->as_process[proc_idx].i4_cur_pic_buf_id,
                                 BUF_MGR_DISP);

 /* Insert the current picture as short term reference */
        ihevc_dpb_mgr_insert_ref((dpb_mgr_t *)ps_codec->pv_dpb_mgr,
                                 ps_codec->as_process[proc_idx].ps_cur_pic,
                                 ps_codec->as_process[proc_idx].i4_cur_pic_buf_id);

 /* If a frame was displayed (in non-shared mode), then release it from display manager */
 if((0 == ps_codec->i4_share_disp_buf) && (ps_codec->ps_disp_buf))
            ihevc_buf_mgr_release((buf_mgr_t *)ps_codec->pv_pic_buf_mgr,
                                  ps_codec->i4_disp_buf_id, BUF_MGR_DISP);

 /* Wait for threads */
 for(i = 0; i < (ps_codec->i4_num_cores - 1); i++)
 {
 if(ps_codec->ai4_process_thread_created[i])
 {
                ithread_join(ps_codec->apv_process_thread_handle[i], NULL);
                ps_codec->ai4_process_thread_created[i] = 0;
 }
 }

        DEBUG_VALIDATE_PADDED_REGION(&ps_codec->as_process[proc_idx]);
 if(ps_codec->u4_pic_cnt > 0)
 {
            DEBUG_DUMP_PIC_PU(ps_codec);
 }
        DEBUG_DUMP_PIC_BUFFERS(ps_codec);

 /* Increment the number of pictures decoded */
        ps_codec->u4_pic_cnt++;
 }
    ihevcd_fill_outargs(ps_codec, ps_dec_ip, ps_dec_op);

 if(1 == ps_dec_op->u4_output_present)
 {
        WORD32 xpos = ps_codec->i4_disp_wd - 32 - LOGO_WD;
        WORD32 ypos = ps_codec->i4_disp_ht - 32 - LOGO_HT;

 if(ypos < 0)
            ypos = 0;

 if(xpos < 0)
            xpos = 0;

        INSERT_LOGO(ps_dec_ip->s_out_buffer.pu1_bufs[0],
                    ps_dec_ip->s_out_buffer.pu1_bufs[1],
                    ps_dec_ip->s_out_buffer.pu1_bufs[2], ps_codec->i4_disp_strd,
                    xpos,
                    ypos,
                    ps_codec->e_chroma_fmt,
                    ps_codec->i4_disp_wd,
                    ps_codec->i4_disp_ht);
 }


 return ret;
}
