void smp_proc_pair_fail(tSMP_CB* p_cb, tSMP_INT_DATA* p_data) {
  SMP_TRACE_DEBUG("%s", __func__);
  p_cb->status = p_data->status;

 /* Cancel pending auth complete timer if set */
  alarm_cancel(p_cb->delayed_auth_timer_ent);
}
