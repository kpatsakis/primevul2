int XfrmController::fillTransportModeUserSpInfo(const XfrmSaInfo& record, XfrmDirection direction,
                                                xfrm_userpolicy_info* usersp) {
    fillXfrmSelector(record, &usersp->sel);
    fillXfrmLifetimeDefaults(&usersp->lft);
    fillXfrmCurLifetimeDefaults(&usersp->curlft);
 /* if (index) index & 0x3 == dir -- must be true
     * xfrm_user.c:verify_newpolicy_info() */
    usersp->index = 0;
    usersp->dir = static_cast<uint8_t>(direction);
    usersp->action = XFRM_POLICY_ALLOW;
    usersp->flags = XFRM_POLICY_LOCALOK;
    usersp->share = XFRM_SHARE_UNIQUE;
 return sizeof(*usersp);
}
