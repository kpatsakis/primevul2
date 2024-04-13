int XfrmController::fillUserSaInfo(const XfrmSaInfo& record, xfrm_usersa_info* usersa) {
    fillXfrmSelector(record, &usersa->sel);

    usersa->id.proto = IPPROTO_ESP;
    usersa->id.spi = record.spi;
    usersa->id.daddr = record.dstAddr;

    usersa->saddr = record.srcAddr;

    fillXfrmLifetimeDefaults(&usersa->lft);
    fillXfrmCurLifetimeDefaults(&usersa->curlft);
    memset(&usersa->stats, 0, sizeof(usersa->stats)); // leave stats zeroed out
    usersa->reqid = record.transformId;
    usersa->family = record.addrFamily;
    usersa->mode = static_cast<uint8_t>(record.mode);
    usersa->replay_window = REPLAY_WINDOW_SIZE;

 if (record.mode == XfrmMode::TRANSPORT) {
        usersa->flags = 0; // TODO: should we actually set flags, XFRM_SA_XFLAG_DONT_ENCAP_DSCP?
 } else {
        usersa->flags = XFRM_STATE_AF_UNSPEC;
 }

 return sizeof(*usersa);
}
