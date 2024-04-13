int XfrmController::fillUserTemplate(const XfrmSaInfo& record, xfrm_user_tmpl* tmpl) {
    tmpl->id.daddr = record.dstAddr;
    tmpl->id.spi = record.spi;
    tmpl->id.proto = IPPROTO_ESP;

    tmpl->family = record.addrFamily;
    tmpl->saddr = record.srcAddr;
    tmpl->reqid = record.transformId;
    tmpl->mode = static_cast<uint8_t>(record.mode);
    tmpl->share = XFRM_SHARE_UNIQUE;
    tmpl->optional = 0; // if this is true, then a failed state lookup will be considered OK:
    tmpl->aalgos = ALGO_MASK_AUTH_ALL; // TODO: if there's a bitmask somewhere of
    tmpl->ealgos = ALGO_MASK_CRYPT_ALL; // TODO: if there's a bitmask somewhere...
 return sizeof(xfrm_user_tmpl*);
}
