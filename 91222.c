netdutils::Status XfrmController::deleteSecurityAssociation(const XfrmId& record,
 const XfrmSocket& sock) {
    xfrm_usersa_id said{};
    nlattr_xfrm_mark xfrmmark{};

 enum { NLMSG_HDR, USERSAID, USERSAID_PAD, MARK, MARK_PAD };

    std::vector<iovec> iov = {
 {NULL, 0}, // reserved for the eventual addition of a NLMSG_HDR
 {&said, 0}, // main usersa_info struct
 {kPadBytes, 0}, // up to NLMSG_ALIGNTO pad bytes of padding
 {&xfrmmark, 0}, // adjust size if xfrm mark is present
 {kPadBytes, 0}, // up to NLATTR_ALIGNTO pad bytes
 };

 int len;
    len = iov[USERSAID].iov_len = fillUserSaId(record, &said);
    iov[USERSAID_PAD].iov_len = NLMSG_ALIGN(len) - len;

    len = iov[MARK].iov_len = fillNlAttrXfrmMark(record, &xfrmmark);
    iov[MARK_PAD].iov_len = NLA_ALIGN(len) - len;

 return sock.sendMessage(XFRM_MSG_DELSA, NETLINK_REQUEST_FLAGS, 0, &iov);
}
