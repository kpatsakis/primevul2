int XfrmController::fillNlAttrXfrmMark(const XfrmId& record, nlattr_xfrm_mark* mark) {
    mark->mark.v = record.mark.v; // set to 0 if it's not used
    mark->mark.m = record.mark.m; // set to 0 if it's not used
 int len = NLA_HDRLEN + sizeof(xfrm_mark);
    fillXfrmNlaHdr(&mark->hdr, XFRMA_MARK, len);
 return len;
}
