void smp_br_select_next_key(tSMP_CB* p_cb, tSMP_INT_DATA* p_data) {
  SMP_TRACE_DEBUG("%s role=%d (0-master) r_keys=0x%x i_keys=0x%x", __func__,
                  p_cb->role, p_cb->local_r_key, p_cb->local_i_key);

 if (p_cb->role == HCI_ROLE_SLAVE ||
 (!p_cb->local_r_key && p_cb->role == HCI_ROLE_MASTER)) {
    smp_key_pick_key(p_cb, p_data);
 }

 if (!p_cb->local_i_key && !p_cb->local_r_key) {
 /* state check to prevent re-entrance */
 if (smp_get_br_state() == SMP_BR_STATE_BOND_PENDING) {
 if (p_cb->total_tx_unacked == 0) {
        tSMP_INT_DATA smp_int_data;
        smp_int_data.status = SMP_SUCCESS;
        smp_br_state_machine_event(p_cb, SMP_BR_AUTH_CMPL_EVT, &smp_int_data);
 } else {
        p_cb->wait_for_authorization_complete = true;
 }
 }
 }
}
