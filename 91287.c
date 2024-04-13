void smp_proc_sec_grant(tSMP_CB* p_cb, tSMP_INT_DATA* p_data) {
 uint8_t res = p_data->status;
  SMP_TRACE_DEBUG("%s", __func__);
 if (res != SMP_SUCCESS) {
    smp_sm_event(p_cb, SMP_AUTH_CMPL_EVT, p_data);
 } else /*otherwise, start pairing */
 {
 /* send IO request callback */
    p_cb->cb_evt = SMP_IO_CAP_REQ_EVT;
 }
}
