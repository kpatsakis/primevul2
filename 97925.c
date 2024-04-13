static int h264_handle_packet_fu_a(AVFormatContext *ctx, PayloadContext *data, AVPacket *pkt,
                                   const uint8_t *buf, int len,
                                   int *nal_counters, int nal_mask)
{
    uint8_t fu_indicator, fu_header, start_bit, nal_type, nal;

    if (len < 3) {
        av_log(ctx, AV_LOG_ERROR, "Too short data for FU-A H.264 RTP packet\n");
        return AVERROR_INVALIDDATA;
    }

    fu_indicator = buf[0];
    fu_header    = buf[1];
    start_bit    = fu_header >> 7;
    nal_type     = fu_header & 0x1f;
    nal          = fu_indicator & 0xe0 | nal_type;

    buf += 2;
    len -= 2;

    if (start_bit && nal_counters)
        nal_counters[nal_type & nal_mask]++;
    return ff_h264_handle_frag_packet(pkt, buf, len, start_bit, &nal, 1);
}
