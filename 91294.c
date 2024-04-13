void smp_process_pairing_commitment(tSMP_CB* p_cb, tSMP_INT_DATA* p_data) {
 uint8_t* p = p_data->p_data;

  SMP_TRACE_DEBUG("%s", __func__);

 if (smp_command_has_invalid_parameters(p_cb)) {
    tSMP_INT_DATA smp_int_data;
    smp_int_data.status = SMP_INVALID_PARAMETERS;
    smp_sm_event(p_cb, SMP_AUTH_CMPL_EVT, &smp_int_data);
 return;
 }

  p_cb->flags |= SMP_PAIR_FLAG_HAVE_PEER_COMM;

 if (p != NULL) {
    STREAM_TO_ARRAY(p_cb->remote_commitment, p, BT_OCTET16_LEN);
 }
}
