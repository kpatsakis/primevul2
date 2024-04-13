static av_cold int rl2_read_header(AVFormatContext *s)
{
    AVIOContext *pb = s->pb;
    AVStream *st;
    unsigned int frame_count;
    unsigned int audio_frame_counter = 0;
    unsigned int video_frame_counter = 0;
    unsigned int back_size;
    unsigned short sound_rate;
    unsigned short rate;
    unsigned short channels;
    unsigned short def_sound_size;
    unsigned int signature;
    unsigned int pts_den = 11025; /* video only case */
    unsigned int pts_num = 1103;
    unsigned int* chunk_offset = NULL;
    int* chunk_size = NULL;
    int* audio_size = NULL;
    int i;
    int ret = 0;

    avio_skip(pb,4);          /* skip FORM tag */
    back_size = avio_rl32(pb); /**< get size of the background frame */
    signature = avio_rb32(pb);
    avio_skip(pb, 4);         /* data size */
    frame_count = avio_rl32(pb);

    /* disallow back_sizes and frame_counts that may lead to overflows later */
    if(back_size > INT_MAX/2  || frame_count > INT_MAX / sizeof(uint32_t))
        return AVERROR_INVALIDDATA;

    avio_skip(pb, 2);         /* encoding method */
    sound_rate = avio_rl16(pb);
    rate = avio_rl16(pb);
    channels = avio_rl16(pb);
    def_sound_size = avio_rl16(pb);

    /** setup video stream */
    st = avformat_new_stream(s, NULL);
    if(!st)
         return AVERROR(ENOMEM);

    st->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
    st->codecpar->codec_id = AV_CODEC_ID_RL2;
    st->codecpar->codec_tag = 0;  /* no fourcc */
    st->codecpar->width = 320;
    st->codecpar->height = 200;

    /** allocate and fill extradata */
    st->codecpar->extradata_size = EXTRADATA1_SIZE;

    if(signature == RLV3_TAG && back_size > 0)
        st->codecpar->extradata_size += back_size;

    if(ff_get_extradata(s, st->codecpar, pb, st->codecpar->extradata_size) < 0)
        return AVERROR(ENOMEM);

    /** setup audio stream if present */
    if(sound_rate){
        if (!channels || channels > 42) {
            av_log(s, AV_LOG_ERROR, "Invalid number of channels: %d\n", channels);
            return AVERROR_INVALIDDATA;
        }

        pts_num = def_sound_size;
        pts_den = rate;

        st = avformat_new_stream(s, NULL);
        if (!st)
            return AVERROR(ENOMEM);
        st->codecpar->codec_type = AVMEDIA_TYPE_AUDIO;
        st->codecpar->codec_id = AV_CODEC_ID_PCM_U8;
        st->codecpar->codec_tag = 1;
        st->codecpar->channels = channels;
        st->codecpar->bits_per_coded_sample = 8;
        st->codecpar->sample_rate = rate;
        st->codecpar->bit_rate = st->codecpar->channels * st->codecpar->sample_rate *
            st->codecpar->bits_per_coded_sample;
        st->codecpar->block_align = st->codecpar->channels *
            st->codecpar->bits_per_coded_sample / 8;
        avpriv_set_pts_info(st,32,1,rate);
    }

    avpriv_set_pts_info(s->streams[0], 32, pts_num, pts_den);

    chunk_size =   av_malloc(frame_count * sizeof(uint32_t));
    audio_size =   av_malloc(frame_count * sizeof(uint32_t));
    chunk_offset = av_malloc(frame_count * sizeof(uint32_t));

    if(!chunk_size || !audio_size || !chunk_offset){
        av_free(chunk_size);
        av_free(audio_size);
        av_free(chunk_offset);
        return AVERROR(ENOMEM);
     }
 
     /** read offset and size tables */
    for(i=0; i < frame_count;i++) {
        if (avio_feof(pb))
            return AVERROR_INVALIDDATA;
         chunk_size[i] = avio_rl32(pb);
    }
    for(i=0; i < frame_count;i++) {
        if (avio_feof(pb))
            return AVERROR_INVALIDDATA;
         chunk_offset[i] = avio_rl32(pb);
    }
    for(i=0; i < frame_count;i++) {
        if (avio_feof(pb))
            return AVERROR_INVALIDDATA;
         audio_size[i] = avio_rl32(pb) & 0xFFFF;
    }
 
     /** build the sample index */
     for(i=0;i<frame_count;i++){
        if(chunk_size[i] < 0 || audio_size[i] > chunk_size[i]){
            ret = AVERROR_INVALIDDATA;
            break;
        }

        if(sound_rate && audio_size[i]){
            av_add_index_entry(s->streams[1], chunk_offset[i],
                audio_frame_counter,audio_size[i], 0, AVINDEX_KEYFRAME);
            audio_frame_counter += audio_size[i] / channels;
        }
        av_add_index_entry(s->streams[0], chunk_offset[i] + audio_size[i],
            video_frame_counter,chunk_size[i]-audio_size[i],0,AVINDEX_KEYFRAME);
        ++video_frame_counter;
    }


    av_free(chunk_size);
    av_free(audio_size);
    av_free(chunk_offset);

    return ret;
}