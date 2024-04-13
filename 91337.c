void smp_process_peer_nonce(tSMP_CB* p_cb, tSMP_INT_DATA* p_data) {
 uint8_t reason;

  SMP_TRACE_DEBUG("%s start ", __func__);

 if (p_cb->cert_failure == SMP_CONFIRM_VALUE_ERR) {
    SMP_TRACE_ERROR("%s failure case = %d", __func__, p_cb->cert_failure);
    reason = p_cb->failure = SMP_CONFIRM_VALUE_ERR;
    smp_sm_event(p_cb, SMP_AUTH_CMPL_EVT, &reason);
 return;
 }
 if ((p_cb->cert_failure == SMP_NUMERIC_COMPAR_FAIL) &&
 (p_cb->selected_association_model == SMP_MODEL_SEC_CONN_JUSTWORKS) &&
 (p_cb->role == HCI_ROLE_SLAVE)) {
    SMP_TRACE_ERROR("%s failure case = %d", __func__, p_cb->cert_failure);
    reason = p_cb->failure = SMP_NUMERIC_COMPAR_FAIL;
    smp_sm_event(p_cb, SMP_AUTH_CMPL_EVT, &reason);
 return;
 }

 switch (p_cb->selected_association_model) {
 case SMP_MODEL_SEC_CONN_JUSTWORKS:
 case SMP_MODEL_SEC_CONN_NUM_COMP:
 /* in these models only master receives commitment */
 if (p_cb->role == HCI_ROLE_MASTER) {
 if (!smp_check_commitment(p_cb)) {
          reason = p_cb->failure = SMP_CONFIRM_VALUE_ERR;
          smp_sm_event(p_cb, SMP_AUTH_CMPL_EVT, &reason);
 break;
 }
 } else {
 /* slave sends local nonce */
        smp_send_rand(p_cb, NULL);
 }

 if (p_cb->selected_association_model == SMP_MODEL_SEC_CONN_JUSTWORKS) {
 /* go directly to phase 2 */
        smp_sm_event(p_cb, SMP_SC_PHASE1_CMPLT_EVT, NULL);
 } else /* numeric comparison */
 {
        smp_set_state(SMP_STATE_WAIT_NONCE);
        smp_sm_event(p_cb, SMP_SC_CALC_NC_EVT, NULL);
 }
 break;
 case SMP_MODEL_SEC_CONN_PASSKEY_ENT:
 case SMP_MODEL_SEC_CONN_PASSKEY_DISP:
 if (!smp_check_commitment(p_cb) &&
          p_cb->cert_failure != SMP_NUMERIC_COMPAR_FAIL) {
        reason = p_cb->failure = SMP_CONFIRM_VALUE_ERR;
        smp_sm_event(p_cb, SMP_AUTH_CMPL_EVT, &reason);
 break;
 }

 if (p_cb->role == HCI_ROLE_SLAVE) {
        smp_send_rand(p_cb, NULL);
 }

 if (++p_cb->round < 20) {
        smp_set_state(SMP_STATE_SEC_CONN_PHS1_START);
        p_cb->flags &= ~SMP_PAIR_FLAG_HAVE_PEER_COMM;
        smp_start_nonce_generation(p_cb);
 break;
 }

      smp_sm_event(p_cb, SMP_SC_PHASE1_CMPLT_EVT, NULL);
 break;
 case SMP_MODEL_SEC_CONN_OOB:
 if (p_cb->role == HCI_ROLE_SLAVE) {
        smp_send_rand(p_cb, NULL);
 }

      smp_sm_event(p_cb, SMP_SC_PHASE1_CMPLT_EVT, NULL);
 break;
 default:
      SMP_TRACE_ERROR("Association Model = %d is not used in LE SC",
                      p_cb->selected_association_model);
 break;
 }

  SMP_TRACE_DEBUG("%s end ", __func__);
}
