static void bta_av_rc_msg_cback(uint8_t handle, uint8_t label, uint8_t opcode,
                                tAVRC_MSG* p_msg) {
 uint8_t* p_data_src = NULL;
 uint16_t data_len = 0;

  APPL_TRACE_DEBUG("%s: handle: %u opcode=0x%x", __func__, handle, opcode);

 /* Copy avrc packet into BTA message buffer (for sending to BTA state machine)
   */

 /* Get size of payload data  (for vendor and passthrough messages only; for
   * browsing
   * messages, use zero-copy) */
 if (opcode == AVRC_OP_VENDOR && p_msg->vendor.p_vendor_data != NULL) {
    p_data_src = p_msg->vendor.p_vendor_data;
    data_len = (uint16_t)p_msg->vendor.vendor_len;
 } else if (opcode == AVRC_OP_PASS_THRU && p_msg->pass.p_pass_data != NULL) {
    p_data_src = p_msg->pass.p_pass_data;
    data_len = (uint16_t)p_msg->pass.pass_len;
 }

 /* Create a copy of the message */
  tBTA_AV_RC_MSG* p_buf =
 (tBTA_AV_RC_MSG*)osi_malloc(sizeof(tBTA_AV_RC_MSG) + data_len);

  p_buf->hdr.event = BTA_AV_AVRC_MSG_EVT;
  p_buf->handle = handle;
  p_buf->label = label;
  p_buf->opcode = opcode;
  memcpy(&p_buf->msg, p_msg, sizeof(tAVRC_MSG));
 /* Copy the data payload, and set the pointer to it */
 if (p_data_src != NULL) {
 uint8_t* p_data_dst = (uint8_t*)(p_buf + 1);
    memcpy(p_data_dst, p_data_src, data_len);

 /* Update bta message buffer to point to payload data */
 /* (Note AVRC_OP_BROWSING uses zero-copy: p_buf->msg.browse.p_browse_data
     * already points to original avrc buffer) */
 if (opcode == AVRC_OP_VENDOR)
      p_buf->msg.vendor.p_vendor_data = p_data_dst;
 else if (opcode == AVRC_OP_PASS_THRU)
      p_buf->msg.pass.p_pass_data = p_data_dst;
 }

 if (opcode == AVRC_OP_BROWSE) {
 /* set p_pkt to NULL, so avrc would not free the buffer */
    p_msg->browse.p_browse_pkt = NULL;
 }

  bta_sys_sendmsg(p_buf);
}
