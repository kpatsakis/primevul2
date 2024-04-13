void smp_process_secure_connection_oob_data(tSMP_CB* p_cb,
                                            tSMP_INT_DATA* p_data) {
  SMP_TRACE_DEBUG("%s", __func__);

  tSMP_SC_OOB_DATA* p_sc_oob_data = &p_cb->sc_oob_data;
 if (p_sc_oob_data->loc_oob_data.present) {
    memcpy(p_cb->local_random, p_sc_oob_data->loc_oob_data.randomizer,
 sizeof(p_cb->local_random));
 } else {
    SMP_TRACE_EVENT("%s: local OOB randomizer is absent", __func__);
    memset(p_cb->local_random, 0, sizeof(p_cb->local_random));
 }

 if (!p_sc_oob_data->peer_oob_data.present) {
    SMP_TRACE_EVENT("%s: peer OOB data is absent", __func__);
    memset(p_cb->peer_random, 0, sizeof(p_cb->peer_random));
 } else {
    memcpy(p_cb->peer_random, p_sc_oob_data->peer_oob_data.randomizer,
 sizeof(p_cb->peer_random));
    memcpy(p_cb->remote_commitment, p_sc_oob_data->peer_oob_data.commitment,
 sizeof(p_cb->remote_commitment));

 uint8_t reason = SMP_CONFIRM_VALUE_ERR;
 /* check commitment */
 if (!smp_check_commitment(p_cb)) {
      p_cb->failure = reason;
      smp_sm_event(p_cb, SMP_AUTH_CMPL_EVT, &reason);
 return;
 }

 if (p_cb->peer_oob_flag != SMP_OOB_PRESENT) {
 /* the peer doesn't have local randomiser */
      SMP_TRACE_EVENT(
 "%s: peer didn't receive local OOB data, set local randomizer to 0",
          __func__);
      memset(p_cb->local_random, 0, sizeof(p_cb->local_random));
 }
 }

  print128(p_cb->local_random, (const uint8_t*)"local OOB randomizer");
  print128(p_cb->peer_random, (const uint8_t*)"peer OOB randomizer");
  smp_start_nonce_generation(p_cb);
}
