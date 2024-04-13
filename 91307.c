void smp_send_pair_req(tSMP_CB* p_cb, tSMP_INT_DATA* p_data) {
  tBTM_SEC_DEV_REC* p_dev_rec = btm_find_dev(p_cb->pairing_bda);
  SMP_TRACE_DEBUG("%s", __func__);

 /* erase all keys when master sends pairing req*/
 if (p_dev_rec) btm_sec_clear_ble_keys(p_dev_rec);
 /* do not manipulate the key, let app decide,
     leave out to BTM to mandate key distribution for bonding case */
  smp_send_cmd(SMP_OPCODE_PAIRING_REQ, p_cb);
}
