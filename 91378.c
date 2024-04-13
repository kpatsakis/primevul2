uint16_t AVRC_MsgReq(uint8_t handle, uint8_t label, uint8_t ctype,
                     BT_HDR* p_pkt) {
 uint8_t* p_data;
 uint8_t cr = AVCT_CMD;
 bool chk_frag = true;
 uint8_t* p_start = NULL;
  tAVRC_FRAG_CB* p_fcb;
 uint16_t len;
 uint16_t status;
 uint8_t msg_mask = 0;
 uint16_t peer_mtu;

 if (!p_pkt) return AVRC_BAD_PARAM;

  AVRC_TRACE_DEBUG("%s handle = %u label = %u ctype = %u len = %d", __func__,
                   handle, label, ctype, p_pkt->len);

 if (ctype >= AVRC_RSP_NOT_IMPL) cr = AVCT_RSP;

 if (p_pkt->event == AVRC_OP_VENDOR) {
 /* add AVRCP Vendor Dependent headers */
    p_start = ((uint8_t*)(p_pkt + 1) + p_pkt->offset);
    p_pkt->offset -= AVRC_VENDOR_HDR_SIZE;
    p_pkt->len += AVRC_VENDOR_HDR_SIZE;
    p_data = (uint8_t*)(p_pkt + 1) + p_pkt->offset;
 *p_data++ = (ctype & AVRC_CTYPE_MASK);
 *p_data++ = (AVRC_SUB_PANEL << AVRC_SUBTYPE_SHIFT);
 *p_data++ = AVRC_OP_VENDOR;
    AVRC_CO_ID_TO_BE_STREAM(p_data, AVRC_CO_METADATA);

 /* Check if this is a AVRC_PDU_REQUEST_CONTINUATION_RSP */
 if (cr == AVCT_CMD) {
      msg_mask |= AVRC_MSG_MASK_IS_VENDOR_CMD;

 if ((*p_start == AVRC_PDU_REQUEST_CONTINUATION_RSP) ||
 (*p_start == AVRC_PDU_ABORT_CONTINUATION_RSP)) {
        msg_mask |= AVRC_MSG_MASK_IS_CONTINUATION_RSP;
 }
 }
 } else if (p_pkt->event == AVRC_OP_PASS_THRU) {
 /* add AVRCP Pass Through headers */
    p_start = ((uint8_t*)(p_pkt + 1) + p_pkt->offset);
    p_pkt->offset -= AVRC_PASS_THRU_SIZE;
    p_pkt->len += AVRC_PASS_THRU_SIZE;
    p_data = (uint8_t*)(p_pkt + 1) + p_pkt->offset;
 *p_data++ = (ctype & AVRC_CTYPE_MASK);
 *p_data++ = (AVRC_SUB_PANEL << AVRC_SUBTYPE_SHIFT);
 *p_data++ = AVRC_OP_PASS_THRU; /* opcode */
 *p_data++ = AVRC_ID_VENDOR; /* operation id */
 *p_data++ = 5; /* operation data len */
    AVRC_CO_ID_TO_BE_STREAM(p_data, AVRC_CO_METADATA);
 } else {
    chk_frag = false;
    peer_mtu = AVCT_GetBrowseMtu(handle);
 if (p_pkt->len > (peer_mtu - AVCT_HDR_LEN_SINGLE)) {
      AVRC_TRACE_ERROR(
 "%s bigger than peer mtu (p_pkt->len(%d) > peer_mtu(%d-%d))",
          __func__, p_pkt->len, peer_mtu, AVCT_HDR_LEN_SINGLE);
      osi_free(p_pkt);
 return AVRC_MSG_TOO_BIG;
 }
 }

 /* abandon previous fragments */
  p_fcb = &avrc_cb.fcb[handle];

 if (p_fcb == NULL) {
    AVRC_TRACE_ERROR("%s p_fcb is NULL", __func__);
    osi_free(p_pkt);
 return AVRC_NOT_OPEN;
 }

 if (p_fcb->frag_enabled) p_fcb->frag_enabled = false;

  osi_free_and_reset((void**)&p_fcb->p_fmsg);

 /* AVRCP spec has not defined any control channel commands that needs
   * fragmentation at this level
   * check for fragmentation only on the response */
 if ((cr == AVCT_RSP) && (chk_frag)) {
 if (p_pkt->len > AVRC_MAX_CTRL_DATA_LEN) {
 int offset_len = MAX(AVCT_MSG_OFFSET, p_pkt->offset);
      BT_HDR* p_pkt_new =
 (BT_HDR*)osi_malloc(AVRC_PACKET_LEN + offset_len + BT_HDR_SIZE);
 if (p_start != NULL) {
        p_fcb->frag_enabled = true;
        p_fcb->p_fmsg = p_pkt;
        p_fcb->frag_pdu = *p_start;
        p_pkt = p_pkt_new;
        p_pkt_new = p_fcb->p_fmsg;
        p_pkt->len = AVRC_MAX_CTRL_DATA_LEN;
        p_pkt->offset = p_pkt_new->offset;
        p_pkt->layer_specific = p_pkt_new->layer_specific;
        p_pkt->event = p_pkt_new->event;
        p_data = (uint8_t*)(p_pkt + 1) + p_pkt->offset;
        p_start -= AVRC_VENDOR_HDR_SIZE;
        memcpy(p_data, p_start, AVRC_MAX_CTRL_DATA_LEN);
 /* use AVRC start packet type */
        p_data += AVRC_VENDOR_HDR_SIZE;
        p_data++; /* pdu */
 *p_data++ = AVRC_PKT_START;

 /* 4 pdu, pkt_type & len */
        len = (AVRC_MAX_CTRL_DATA_LEN - AVRC_VENDOR_HDR_SIZE -
               AVRC_MIN_META_HDR_SIZE);
        UINT16_TO_BE_STREAM(p_data, len);

 /* prepare the left over for as an end fragment */
        avrc_prep_end_frag(handle);
        AVRC_TRACE_DEBUG("%s p_pkt len:%d/%d, next len:%d", __func__,
                         p_pkt->len, len, p_fcb->p_fmsg->len);
 } else {
 /* TODO: Is this "else" block valid? Remove it? */
        AVRC_TRACE_ERROR("%s no buffers for fragmentation", __func__);
        osi_free(p_pkt);
 return AVRC_NO_RESOURCES;
 }
 }
 } else if ((p_pkt->event == AVRC_OP_VENDOR) && (cr == AVCT_CMD) &&
 (avrc_cb.ccb_int[handle].flags & AVRC_CB_FLAGS_RSP_PENDING) &&
 !(msg_mask & AVRC_MSG_MASK_IS_CONTINUATION_RSP)) {
 /* If we are sending a vendor specific command, and a response is pending,
     * then enqueue the command until the response has been received.
     * This is to interop with TGs that abort sending responses whenever a new
     * command
     * is received (exception is continuation request command
     * must sent that to get additional response frags) */
    AVRC_TRACE_DEBUG(
 "AVRC: Enqueuing command 0x%08x (handle=0x%02x, label=0x%02x)", p_pkt,
        handle, label);

 /* label in BT_HDR (will need this later when the command is dequeued) */
    p_pkt->layer_specific = (label << 8) | (p_pkt->layer_specific & 0xFF);

 /* Enqueue the command */
    fixed_queue_enqueue(avrc_cb.ccb_int[handle].cmd_q, p_pkt);
 return AVRC_SUCCESS;
 }

 /* Send the message */
  status = AVCT_MsgReq(handle, label, cr, p_pkt);
 if ((status == AVCT_SUCCESS) && (cr == AVCT_CMD)) {
 /* If a command was successfully sent, indicate that a response is pending
     */
    avrc_cb.ccb_int[handle].flags |= AVRC_CB_FLAGS_RSP_PENDING;

 /* Start command timer to wait for response */
    avrc_start_cmd_timer(handle, label, msg_mask);
 }

 return status;
}
