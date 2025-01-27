static int mov_write_colr_tag(AVIOContext *pb, MOVTrack *track)
{

    if (track->par->color_primaries == AVCOL_PRI_UNSPECIFIED &&
        track->par->color_trc == AVCOL_TRC_UNSPECIFIED &&
        track->par->color_space == AVCOL_SPC_UNSPECIFIED) {
        if ((track->par->width >= 1920 && track->par->height >= 1080)
          || (track->par->width == 1280 && track->par->height == 720)) {
            av_log(NULL, AV_LOG_WARNING, "color primaries unspecified, assuming bt709\n");
            track->par->color_primaries = AVCOL_PRI_BT709;
        } else if (track->par->width == 720 && track->height == 576) {
            av_log(NULL, AV_LOG_WARNING, "color primaries unspecified, assuming bt470bg\n");
            track->par->color_primaries = AVCOL_PRI_BT470BG;
        } else if (track->par->width == 720 &&
                   (track->height == 486 || track->height == 480)) {
            av_log(NULL, AV_LOG_WARNING, "color primaries unspecified, assuming smpte170\n");
            track->par->color_primaries = AVCOL_PRI_SMPTE170M;
        } else {
            av_log(NULL, AV_LOG_WARNING, "color primaries unspecified, unable to assume anything\n");
        }
        switch (track->par->color_primaries) {
        case AVCOL_PRI_BT709:
            track->par->color_trc = AVCOL_TRC_BT709;
            track->par->color_space = AVCOL_SPC_BT709;
            break;
        case AVCOL_PRI_SMPTE170M:
        case AVCOL_PRI_BT470BG:
            track->par->color_trc = AVCOL_TRC_BT709;
            track->par->color_space = AVCOL_SPC_SMPTE170M;
            break;
        }
    }

    /* We should only ever be called by MOV or MP4. */
    av_assert0(track->mode == MODE_MOV || track->mode == MODE_MP4);

    avio_wb32(pb, 18 + (track->mode == MODE_MP4));
    ffio_wfourcc(pb, "colr");
    if (track->mode == MODE_MP4)
        ffio_wfourcc(pb, "nclx");
    else
        ffio_wfourcc(pb, "nclc");
    switch (track->par->color_primaries) {
    case AVCOL_PRI_BT709:     avio_wb16(pb, 1); break;
    case AVCOL_PRI_BT470BG:   avio_wb16(pb, 5); break;
    case AVCOL_PRI_SMPTE170M:
    case AVCOL_PRI_SMPTE240M: avio_wb16(pb, 6); break;
    case AVCOL_PRI_BT2020:    avio_wb16(pb, 9); break;
    case AVCOL_PRI_SMPTE431:  avio_wb16(pb, 11); break;
    case AVCOL_PRI_SMPTE432:  avio_wb16(pb, 12); break;
    default:                  avio_wb16(pb, 2);
    }
    switch (track->par->color_trc) {
    case AVCOL_TRC_BT709:        avio_wb16(pb, 1); break;
    case AVCOL_TRC_SMPTE170M:    avio_wb16(pb, 1); break; // remapped
    case AVCOL_TRC_SMPTE240M:    avio_wb16(pb, 7); break;
    case AVCOL_TRC_SMPTEST2084:  avio_wb16(pb, 16); break;
    case AVCOL_TRC_SMPTE428:     avio_wb16(pb, 17); break;
    case AVCOL_TRC_ARIB_STD_B67: avio_wb16(pb, 18); break;
    default:                     avio_wb16(pb, 2);
    }
    switch (track->par->color_space) {
    case AVCOL_SPC_BT709:      avio_wb16(pb, 1); break;
    case AVCOL_SPC_BT470BG:
    case AVCOL_SPC_SMPTE170M:  avio_wb16(pb, 6); break;
    case AVCOL_SPC_SMPTE240M:  avio_wb16(pb, 7); break;
    case AVCOL_SPC_BT2020_NCL: avio_wb16(pb, 9); break;
    default:                   avio_wb16(pb, 2);
    }

    if (track->mode == MODE_MP4) {
        int full_range = track->par->color_range == AVCOL_RANGE_JPEG;
        avio_w8(pb, full_range << 7);
        return 19;
    } else {
        return 18;
    }
}
