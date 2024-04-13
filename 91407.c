static void sdp_snd_service_search_req(tCONN_CB* p_ccb, uint8_t cont_len,
 uint8_t* p_cont) {
 uint8_t *p, *p_start, *p_param_len;
  BT_HDR* p_cmd = (BT_HDR*)osi_malloc(SDP_DATA_BUF_SIZE);
 uint16_t param_len;

 /* Prepare the buffer for sending the packet to L2CAP */
  p_cmd->offset = L2CAP_MIN_OFFSET;
  p = p_start = (uint8_t*)(p_cmd + 1) + L2CAP_MIN_OFFSET;

 /* Build a service search request packet */
  UINT8_TO_BE_STREAM(p, SDP_PDU_SERVICE_SEARCH_REQ);
  UINT16_TO_BE_STREAM(p, p_ccb->transaction_id);
  p_ccb->transaction_id++;

 /* Skip the length, we need to add it at the end */
  p_param_len = p;
  p += 2;

/* Build the UID sequence. */
#if (SDP_BROWSE_PLUS == TRUE)
  p = sdpu_build_uuid_seq(p, 1,
 &p_ccb->p_db->uuid_filters[p_ccb->cur_uuid_idx]);
#else
  p = sdpu_build_uuid_seq(p, p_ccb->p_db->num_uuid_filters,
                          p_ccb->p_db->uuid_filters);
#endif

 /* Set max service record count */
  UINT16_TO_BE_STREAM(p, sdp_cb.max_recs_per_search);

 /* Set continuation state */
  UINT8_TO_BE_STREAM(p, cont_len);

 /* if this is not the first request */
 if (cont_len && p_cont) {
    memcpy(p, p_cont, cont_len);
    p += cont_len;
 }

 /* Go back and put the parameter length into the buffer */
  param_len = (uint16_t)(p - p_param_len - 2);
  UINT16_TO_BE_STREAM(p_param_len, param_len);

  p_ccb->disc_state = SDP_DISC_WAIT_HANDLES;

 /* Set the length of the SDP data in the buffer */
  p_cmd->len = (uint16_t)(p - p_start);

#if (SDP_DEBUG_RAW == TRUE)
  SDP_TRACE_WARNING("sdp_snd_service_search_req cont_len :%d disc_state:%d",
                    cont_len, p_ccb->disc_state);
#endif

  L2CA_DataWrite(p_ccb->connection_id, p_cmd);

 /* Start inactivity timer */
  alarm_set_on_mloop(p_ccb->sdp_conn_timer, SDP_INACT_TIMEOUT_MS,
                     sdp_conn_timer_timeout, p_ccb);
}
