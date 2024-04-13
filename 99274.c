int ff_mpeg_update_thread_context(AVCodecContext *dst,
                                  const AVCodecContext *src)
{
    int i, ret;
    MpegEncContext *s = dst->priv_data, *s1 = src->priv_data;

    if (dst == src)
        return 0;

    av_assert0(s != s1);

    if (!s->context_initialized) {
        int err;
        memcpy(s, s1, sizeof(MpegEncContext));

        s->avctx                 = dst;
        s->bitstream_buffer      = NULL;
        s->bitstream_buffer_size = s->allocated_bitstream_buffer_size = 0;

        if (s1->context_initialized){
            ff_mpv_idct_init(s);
            if((err = ff_mpv_common_init(s)) < 0){
                memset(s, 0, sizeof(MpegEncContext));
                s->avctx = dst;
                return err;
            }
        }
    }

    if (s->height != s1->height || s->width != s1->width || s->context_reinit) {
        s->context_reinit = 0;
        s->height = s1->height;
        s->width  = s1->width;
        if ((ret = ff_mpv_common_frame_size_change(s)) < 0)
            return ret;
    }

    s->avctx->coded_height  = s1->avctx->coded_height;
    s->avctx->coded_width   = s1->avctx->coded_width;
    s->avctx->width         = s1->avctx->width;
    s->avctx->height        = s1->avctx->height;

    s->coded_picture_number = s1->coded_picture_number;
    s->picture_number       = s1->picture_number;

    av_assert0(!s->picture || s->picture != s1->picture);
    if(s->picture)
    for (i = 0; i < MAX_PICTURE_COUNT; i++) {
        ff_mpeg_unref_picture(s->avctx, &s->picture[i]);
        if (s1->picture && s1->picture[i].f->buf[0] &&
            (ret = ff_mpeg_ref_picture(s->avctx, &s->picture[i], &s1->picture[i])) < 0)
            return ret;
    }

#define UPDATE_PICTURE(pic)\
do {\
    ff_mpeg_unref_picture(s->avctx, &s->pic);\
    if (s1->pic.f && s1->pic.f->buf[0])\
        ret = ff_mpeg_ref_picture(s->avctx, &s->pic, &s1->pic);\
    else\
        ret = ff_update_picture_tables(&s->pic, &s1->pic);\
    if (ret < 0)\
        return ret;\
} while (0)

    UPDATE_PICTURE(current_picture);
    UPDATE_PICTURE(last_picture);
    UPDATE_PICTURE(next_picture);

#define REBASE_PICTURE(pic, new_ctx, old_ctx)                                 \
    ((pic && pic >= old_ctx->picture &&                                       \
      pic < old_ctx->picture + MAX_PICTURE_COUNT) ?                           \
        &new_ctx->picture[pic - old_ctx->picture] : NULL)

    s->last_picture_ptr    = REBASE_PICTURE(s1->last_picture_ptr,    s, s1);
    s->current_picture_ptr = REBASE_PICTURE(s1->current_picture_ptr, s, s1);
    s->next_picture_ptr    = REBASE_PICTURE(s1->next_picture_ptr,    s, s1);

    s->next_p_frame_damaged = s1->next_p_frame_damaged;
    s->workaround_bugs      = s1->workaround_bugs;
    s->padding_bug_score    = s1->padding_bug_score;

    memcpy(&s->last_time_base, &s1->last_time_base,
           (char *) &s1->pb_field_time + sizeof(s1->pb_field_time) -
           (char *) &s1->last_time_base);

    s->max_b_frames = s1->max_b_frames;
    s->low_delay    = s1->low_delay;
    s->droppable    = s1->droppable;

    s->divx_packed  = s1->divx_packed;

    if (s1->bitstream_buffer) {
        if (s1->bitstream_buffer_size +
            AV_INPUT_BUFFER_PADDING_SIZE > s->allocated_bitstream_buffer_size) {
            av_fast_malloc(&s->bitstream_buffer,
                           &s->allocated_bitstream_buffer_size,
                           s1->allocated_bitstream_buffer_size);
            if (!s->bitstream_buffer) {
                s->bitstream_buffer_size = 0;
                return AVERROR(ENOMEM);
            }
        }
        s->bitstream_buffer_size = s1->bitstream_buffer_size;
        memcpy(s->bitstream_buffer, s1->bitstream_buffer,
               s1->bitstream_buffer_size);
        memset(s->bitstream_buffer + s->bitstream_buffer_size, 0,
               AV_INPUT_BUFFER_PADDING_SIZE);
    }

    if (!s->sc.edge_emu_buffer)
        if (s1->linesize) {
            if (ff_mpeg_framesize_alloc(s->avctx, &s->me,
                                        &s->sc, s1->linesize) < 0) {
                av_log(s->avctx, AV_LOG_ERROR, "Failed to allocate context "
                       "scratch buffers.\n");
                return AVERROR(ENOMEM);
            }
        } else {
            av_log(s->avctx, AV_LOG_ERROR, "Context scratch buffers could not "
                   "be allocated due to unknown size.\n");
        }

    memcpy(&s->progressive_sequence, &s1->progressive_sequence,
           (char *) &s1->rtp_mode - (char *) &s1->progressive_sequence);

    if (!s1->first_field) {
        s->last_pict_type = s1->pict_type;
        if (s1->current_picture_ptr)
            s->last_lambda_for[s1->pict_type] = s1->current_picture_ptr->f->quality;
    }

    return 0;
}
