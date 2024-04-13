netdutils::Status XfrmController::flushSaDb(const XfrmSocket& s) {
 struct xfrm_usersa_flush flushUserSa = {.proto = IPSEC_PROTO_ANY};

    std::vector<iovec> iov = {{NULL, 0}, // reserved for the eventual addition of a NLMSG_HDR
 {&flushUserSa, sizeof(flushUserSa)}, // xfrm_usersa_flush structure
 {kPadBytes, NLMSG_ALIGN(sizeof(flushUserSa)) - sizeof(flushUserSa)}};

 return s.sendMessage(XFRM_MSG_FLUSHSA, NETLINK_REQUEST_FLAGS, 0, &iov);
}
