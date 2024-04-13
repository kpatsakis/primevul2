void smp_process_io_response(tSMP_CB* p_cb, tSMP_INT_DATA* p_data) {
 uint8_t reason = SMP_PAIR_AUTH_FAIL;

  SMP_TRACE_DEBUG("%s", __func__);
 if (p_cb->flags & SMP_PAIR_FLAGS_WE_STARTED_DD) {
 /* pairing started by local (slave) Security Request */
    smp_set_state(SMP_STATE_SEC_REQ_PENDING);
    smp_send_cmd(SMP_OPCODE_SEC_REQ, p_cb);
 } else /* plan to send pairing respond */
 {
 /* pairing started by peer (master) Pairing Request */
    p_cb->selected_association_model = smp_select_association_model(p_cb);

 if (p_cb->secure_connections_only_mode_required &&
 (!(p_cb->le_secure_connections_mode_is_used) ||
 (p_cb->selected_association_model == SMP_MODEL_SEC_CONN_JUSTWORKS))) {
      SMP_TRACE_ERROR(
 "Slave requires secure connection only mode "
 "but it can't be provided -> Slave fails pairing");
      smp_sm_event(p_cb, SMP_AUTH_CMPL_EVT, &reason);
 return;
 }

 if (p_cb->selected_association_model == SMP_MODEL_SEC_CONN_OOB) {
 if (smp_request_oob_data(p_cb)) return;
 }

 if (pts_test_send_authentication_complete_failure(p_cb)) return;

    smp_send_pair_rsp(p_cb, NULL);
 }
}
