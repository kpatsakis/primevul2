inline int PacketCopyDataOffset(Packet *p, uint32_t offset, uint8_t *data, uint32_t datalen)
{
    if (unlikely(offset + datalen > MAX_PAYLOAD_SIZE)) {
        /* too big */
        return -1;
    }

    /* Do we have already an packet with allocated data */
    if (! p->ext_pkt) {
        uint32_t newsize = offset + datalen;
        if (newsize < offset)
            return -1;
        if (newsize <= default_packet_size) {
            /* data will fit in memory allocated with packet */
            memcpy(GET_PKT_DIRECT_DATA(p) + offset, data, datalen);
        } else {
            /* here we need a dynamic allocation */
            p->ext_pkt = SCMalloc(MAX_PAYLOAD_SIZE);
            if (unlikely(p->ext_pkt == NULL)) {
                SET_PKT_LEN(p, 0);
                return -1;
            }
            /* copy initial data */
            memcpy(p->ext_pkt, GET_PKT_DIRECT_DATA(p), GET_PKT_DIRECT_MAX_SIZE(p));
            /* copy data as asked */
            memcpy(p->ext_pkt + offset, data, datalen);
        }
    } else {
        memcpy(p->ext_pkt + offset, data, datalen);
    }
    return 0;
}
