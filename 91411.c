void rfc_send_dm(tRFC_MCB* p_mcb, uint8_t dlci, bool pf) {
 uint8_t* p_data;
 uint8_t cr = RFCOMM_CR(p_mcb->is_initiator, false);
  BT_HDR* p_buf = (BT_HDR*)osi_malloc(RFCOMM_CMD_BUF_SIZE);

  p_buf->offset = L2CAP_MIN_OFFSET;
  p_data = (uint8_t*)(p_buf + 1) + L2CAP_MIN_OFFSET;

 /* DM frame, response, PF = 1, dlci */
 *p_data++ = RFCOMM_EA | cr | (dlci << RFCOMM_SHIFT_DLCI);
 *p_data++ = RFCOMM_DM | ((pf) ? RFCOMM_PF : 0);
 *p_data++ = RFCOMM_EA | 0;

 *p_data = RFCOMM_DM_FCS((uint8_t*)(p_buf + 1) + L2CAP_MIN_OFFSET, cr, dlci);

  p_buf->len = 4;

  rfc_check_send_cmd(p_mcb, p_buf);
}
