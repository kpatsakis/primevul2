netdutils::Status XfrmController::updateSecurityAssociation(const XfrmSaInfo& record,
 const XfrmSocket& sock) {
    xfrm_usersa_info usersa{};
    nlattr_algo_crypt crypt{};
    nlattr_algo_auth auth{};
    nlattr_algo_aead aead{};
    nlattr_xfrm_mark xfrmmark{};
    nlattr_xfrm_output_mark xfrmoutputmark{};
    nlattr_encap_tmpl encap{};

 enum {
        NLMSG_HDR,
        USERSA,
        USERSA_PAD,
        CRYPT,
        CRYPT_PAD,
        AUTH,
        AUTH_PAD,
        AEAD,
        AEAD_PAD,
        MARK,
        MARK_PAD,
        OUTPUT_MARK,
        OUTPUT_MARK_PAD,
        ENCAP,
        ENCAP_PAD,
 };

    std::vector<iovec> iov = {
 {NULL, 0}, // reserved for the eventual addition of a NLMSG_HDR
 {&usersa, 0}, // main usersa_info struct
 {kPadBytes, 0}, // up to NLMSG_ALIGNTO pad bytes of padding
 {&crypt, 0}, // adjust size if crypt algo is present
 {kPadBytes, 0}, // up to NLATTR_ALIGNTO pad bytes
 {&auth, 0}, // adjust size if auth algo is present
 {kPadBytes, 0}, // up to NLATTR_ALIGNTO pad bytes
 {&aead, 0}, // adjust size if aead algo is present
 {kPadBytes, 0}, // up to NLATTR_ALIGNTO pad bytes
 {&xfrmmark, 0}, // adjust size if xfrm mark is present
 {kPadBytes, 0}, // up to NLATTR_ALIGNTO pad bytes
 {&xfrmoutputmark, 0}, // adjust size if xfrm output mark is present
 {kPadBytes, 0}, // up to NLATTR_ALIGNTO pad bytes
 {&encap, 0}, // adjust size if encapsulating
 {kPadBytes, 0}, // up to NLATTR_ALIGNTO pad bytes
 };

 if (!record.aead.name.empty() && (!record.auth.name.empty() || !record.crypt.name.empty())) {
 return netdutils::statusFromErrno(EINVAL, "Invalid xfrm algo selection; AEAD is mutually "
 "exclusive with both Authentication and "
 "Encryption");
 }

 if (record.aead.key.size() > MAX_KEY_LENGTH || record.auth.key.size() > MAX_KEY_LENGTH ||
        record.crypt.key.size() > MAX_KEY_LENGTH) {
 return netdutils::statusFromErrno(EINVAL, "Key length invalid; exceeds MAX_KEY_LENGTH");
 }

 int len;
    len = iov[USERSA].iov_len = fillUserSaInfo(record, &usersa);
    iov[USERSA_PAD].iov_len = NLMSG_ALIGN(len) - len;

    len = iov[CRYPT].iov_len = fillNlAttrXfrmAlgoEnc(record.crypt, &crypt);
    iov[CRYPT_PAD].iov_len = NLA_ALIGN(len) - len;

    len = iov[AUTH].iov_len = fillNlAttrXfrmAlgoAuth(record.auth, &auth);
    iov[AUTH_PAD].iov_len = NLA_ALIGN(len) - len;

    len = iov[AEAD].iov_len = fillNlAttrXfrmAlgoAead(record.aead, &aead);
    iov[AEAD_PAD].iov_len = NLA_ALIGN(len) - len;

    len = iov[MARK].iov_len = fillNlAttrXfrmMark(record, &xfrmmark);
    iov[MARK_PAD].iov_len = NLA_ALIGN(len) - len;

    len = iov[OUTPUT_MARK].iov_len = fillNlAttrXfrmOutputMark(record.netId, &xfrmoutputmark);
    iov[OUTPUT_MARK_PAD].iov_len = NLA_ALIGN(len) - len;

    len = iov[ENCAP].iov_len = fillNlAttrXfrmEncapTmpl(record, &encap);
    iov[ENCAP_PAD].iov_len = NLA_ALIGN(len) - len;

 return sock.sendMessage(XFRM_MSG_UPDSA, NETLINK_REQUEST_FLAGS, 0, &iov);
}
