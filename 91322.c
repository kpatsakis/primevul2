void smp_link_encrypted(const RawAddress& bda, uint8_t encr_enable) {
  tSMP_CB* p_cb = &smp_cb;

  SMP_TRACE_DEBUG("%s: encr_enable=%d", __func__, encr_enable);

 if (smp_cb.pairing_bda == bda) {
 /* encryption completed with STK, remember the key size now, could be
     * overwritten when key exchange happens                                 */
 if (p_cb->loc_enc_size != 0 && encr_enable) {
 /* update the link encryption key size if a SMP pairing just performed */
      btm_ble_update_sec_key_size(bda, p_cb->loc_enc_size);
 }

    smp_sm_event(&smp_cb, SMP_ENCRYPTED_EVT, &encr_enable);
 }
}
