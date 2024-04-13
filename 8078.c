static int mov_read_pssh(MOVContext *c, AVIOContext *pb, MOVAtom atom)
{
    AVEncryptionInitInfo *info, *old_init_info;
    uint8_t **key_ids;
    AVStream *st;
    uint8_t *side_data, *extra_data, *old_side_data;
    size_t side_data_size, old_side_data_size;
    int ret = 0;
    unsigned int version, kid_count, extra_data_size, alloc_size = 0;

    if (c->fc->nb_streams < 1)
        return 0;
    st = c->fc->streams[c->fc->nb_streams-1];

    version = avio_r8(pb); /* version */
    avio_rb24(pb);  /* flags */

    info = av_encryption_init_info_alloc(/* system_id_size */ 16, /* num_key_ids */ 0,
                                         /* key_id_size */ 16, /* data_size */ 0);
    if (!info)
        return AVERROR(ENOMEM);

    if ((ret = ffio_read_size(pb, info->system_id, 16)) < 0) {
        av_log(c->fc, AV_LOG_ERROR, "Failed to read the system id\n");
        goto finish;
    }

    if (version > 0) {
        kid_count = avio_rb32(pb);
        if (kid_count >= INT_MAX / sizeof(*key_ids)) {
            ret = AVERROR(ENOMEM);
            goto finish;
        }

        for (unsigned int i = 0; i < kid_count && !pb->eof_reached; i++) {
            unsigned int min_kid_count = FFMIN(FFMAX(i + 1, 1024), kid_count);
            key_ids = av_fast_realloc(info->key_ids, &alloc_size,
                                      min_kid_count * sizeof(*key_ids));
            if (!key_ids) {
                ret = AVERROR(ENOMEM);
                goto finish;
            }
            info->key_ids = key_ids;

            info->key_ids[i] = av_mallocz(16);
            if (!info->key_ids[i]) {
                ret = AVERROR(ENOMEM);
                goto finish;
            }
            info->num_key_ids = i + 1;

            if ((ret = ffio_read_size(pb, info->key_ids[i], 16)) < 0) {
                av_log(c->fc, AV_LOG_ERROR, "Failed to read the key id\n");
                goto finish;
            }
        }

        if (pb->eof_reached) {
            av_log(c->fc, AV_LOG_ERROR, "Hit EOF while reading pssh\n");
            ret = AVERROR_INVALIDDATA;
            goto finish;
        }
    }

    extra_data_size = avio_rb32(pb);
    ret = mov_try_read_block(pb, extra_data_size, &extra_data);
    if (ret < 0)
        goto finish;

    av_freep(&info->data);  // malloc(0) may still allocate something.
    info->data = extra_data;
    info->data_size = extra_data_size;

    // If there is existing initialization data, append to the list.
    old_side_data = av_stream_get_side_data(st, AV_PKT_DATA_ENCRYPTION_INIT_INFO, &old_side_data_size);
    if (old_side_data) {
        old_init_info = av_encryption_init_info_get_side_data(old_side_data, old_side_data_size);
        if (old_init_info) {
            // Append to the end of the list.
            for (AVEncryptionInitInfo *cur = old_init_info;; cur = cur->next) {
                if (!cur->next) {
                    cur->next = info;
                    break;
                }
            }
            info = old_init_info;
        } else {
            // Assume existing side-data will be valid, so the only error we could get is OOM.
            ret = AVERROR(ENOMEM);
            goto finish;
        }
    }

    side_data = av_encryption_init_info_add_side_data(info, &side_data_size);
    if (!side_data) {
        ret = AVERROR(ENOMEM);
        goto finish;
    }
    ret = av_stream_add_side_data(st, AV_PKT_DATA_ENCRYPTION_INIT_INFO,
                                  side_data, side_data_size);
    if (ret < 0)
        av_free(side_data);

finish:
    av_encryption_init_info_free(info);
    return ret;
}