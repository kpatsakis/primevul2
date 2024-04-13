WriteSamples(AP4_Mpeg2TsWriter*               ts_writer,
             PackedAudioWriter*               packed_writer,
             AP4_Track*                       audio_track,
             SampleReader*                    audio_reader, 
             AP4_Mpeg2TsWriter::SampleStream* audio_stream,
             AP4_Track*                       video_track,
             SampleReader*                    video_reader, 
             AP4_Mpeg2TsWriter::SampleStream* video_stream,
             unsigned int                     segment_duration_threshold,
             AP4_UI08                         nalu_length_size)
{
    AP4_Sample              audio_sample;
    AP4_DataBuffer          audio_sample_data;
    unsigned int            audio_sample_count = 0;
    double                  audio_ts = 0.0;
    double                  audio_frame_duration = 0.0;
    bool                    audio_eos = false;
    AP4_Sample              video_sample;
    AP4_DataBuffer          video_sample_data;
    unsigned int            video_sample_count = 0;
    double                  video_ts = 0.0;
    double                  video_frame_duration = 0.0;
    bool                    video_eos = false;
    double                  last_ts = 0.0;
    unsigned int            segment_number = 0;
    AP4_ByteStream*         segment_output = NULL;
    double                  segment_duration = 0.0;
    AP4_Array<double>       segment_durations;
    AP4_Array<AP4_UI32>     segment_sizes;
    AP4_Position            segment_position = 0;
    AP4_Array<AP4_Position> segment_positions;
    AP4_Array<AP4_Position> iframe_positions;
    AP4_Array<AP4_UI32>     iframe_sizes;
    AP4_Array<double>       iframe_times;
    AP4_Array<double>       iframe_durations;
    AP4_Array<AP4_UI32>     iframe_segment_indexes;
    bool                    new_segment = true;
    AP4_ByteStream*         raw_output = NULL;
    AP4_ByteStream*         playlist = NULL;
    char                    string_buffer[4096];
    SampleEncrypter*        sample_encrypter = NULL;
    AP4_Result              result = AP4_SUCCESS;
    
    // prime the samples
    if (audio_reader) {
        result = ReadSample(*audio_reader, *audio_track, audio_sample, audio_sample_data, audio_ts, audio_frame_duration, audio_eos);
        if (AP4_FAILED(result)) return result;
    }
    if (video_reader) {
        result = ReadSample(*video_reader, *video_track, video_sample, video_sample_data, video_ts, video_frame_duration, video_eos);
        if (AP4_FAILED(result)) return result;
    }
    
    for (;;) {
        bool sync_sample = false;
        AP4_Track* chosen_track= NULL;
        if (audio_track && !audio_eos) {
            chosen_track = audio_track;
            if (video_track == NULL) {
                if (audio_track->GetSampleDescription(0)->GetFormat() == AP4_SAMPLE_FORMAT_AC_4) {
                    if (audio_sample.IsSync()) {
                        sync_sample = true;
                    }
                } else {
                    sync_sample = true;
                }
            }
        }
        if (video_track && !video_eos) {
            if (audio_track) {
                if (video_ts <= audio_ts) {
                    chosen_track = video_track;
                }
            } else {
                chosen_track = video_track;
            }
            if (chosen_track == video_track && video_sample.IsSync()) {
                sync_sample = true;
            }
        }
        
        // check if we need to start a new segment
        if (Options.segment_duration && (sync_sample || chosen_track == NULL)) {
            if (video_track) {
                segment_duration = video_ts - last_ts;
            } else {
                segment_duration = audio_ts - last_ts;
            }
            if ((segment_duration >= (double)Options.segment_duration - (double)segment_duration_threshold/1000.0) ||
                chosen_track == NULL) {
                if (video_track) {
                    last_ts = video_ts;
                } else {
                    last_ts = audio_ts;
                }
                if (segment_output) {
                    // flush the output stream
                    segment_output->Flush();
                    
                    // compute the segment size (including padding)
                    AP4_Position segment_end = 0;
                    segment_output->Tell(segment_end);
                    AP4_UI32 segment_size = 0;
                    if (Options.encryption_mode == ENCRYPTION_MODE_AES_128) {
                        segment_size = (AP4_UI32)segment_end;
                    } else if (segment_end > segment_position) {
                        segment_size = (AP4_UI32)(segment_end-segment_position);
                    }
                    
                    // update counters
                    segment_sizes.Append(segment_size);
                    segment_positions.Append(segment_position);
                    segment_durations.Append(segment_duration);
            
                    if (segment_duration != 0.0) {
                        double segment_bitrate = 8.0*(double)segment_size/segment_duration;
                        if (segment_bitrate > Stats.max_segment_bitrate) {
                            Stats.max_segment_bitrate = segment_bitrate;
                        }
                    }
                    if (Options.verbose) {
                        printf("Segment %d, duration=%.2f, %d audio samples, %d video samples, %d bytes @%lld\n",
                               segment_number, 
                               segment_duration,
                               audio_sample_count, 
                               video_sample_count,
                               segment_size,
                               segment_position);
                    }
                    if (!Options.output_single_file) {
                        segment_output->Release();
                        segment_output = NULL;
                    }
                    ++segment_number;
                    audio_sample_count = 0;
                    video_sample_count = 0;
                }
                new_segment = true;
            }
        }

        // check if we're done
        if (chosen_track == NULL) break;
        
        if (new_segment) {
            new_segment = false;
            
            // compute the new segment position
            segment_position = 0;
            if (Options.output_single_file) {
                if (raw_output) {
                    raw_output->Tell(segment_position);
                }
            }
            
            // manage the new segment stream
            if (segment_output == NULL) {
                segment_output = OpenOutput(Options.segment_filename_template, segment_number);
                raw_output = segment_output;
                if (segment_output == NULL) return AP4_ERROR_CANNOT_OPEN_FILE;
            }
            if (Options.encryption_mode != ENCRYPTION_MODE_NONE) {
                if (Options.encryption_iv_mode == ENCRYPTION_IV_MODE_SEQUENCE) {
                    AP4_SetMemory(Options.encryption_iv, 0, sizeof(Options.encryption_iv));
                    AP4_BytesFromUInt32BE(&Options.encryption_iv[12], segment_number);
                }
            }
            if (Options.encryption_mode == ENCRYPTION_MODE_AES_128) {
                EncryptingStream* encrypting_stream = NULL;
                result = EncryptingStream::Create(Options.encryption_key, Options.encryption_iv, raw_output, encrypting_stream);
                if (AP4_FAILED(result)) {
                    fprintf(stderr, "ERROR: failed to create encrypting stream (%d)\n", result);
                    return result;
                }
                segment_output->Release();
                segment_output = encrypting_stream;
            } else if (Options.encryption_mode == ENCRYPTION_MODE_SAMPLE_AES) {
                delete sample_encrypter;
                sample_encrypter = NULL;
                result = SampleEncrypter::Create(Options.encryption_key, Options.encryption_iv, sample_encrypter);
                if (AP4_FAILED(result)) {
                    fprintf(stderr, "ERROR: failed to create sample encrypter (%d)\n", result);
                    return result;
                }
            }
            
            // write the PAT and PMT
            if (ts_writer) {
                // update the descriptors if needed
                if (Options.encryption_mode == ENCRYPTION_MODE_SAMPLE_AES) {
                    AP4_DataBuffer descriptor;
                    if (audio_track) {
                        result = MakeSampleAesAudioDescriptor(descriptor, audio_track->GetSampleDescription(0), audio_sample_data);
                        if (AP4_SUCCEEDED(result) && descriptor.GetDataSize()) {
                            audio_stream->SetDescriptor(descriptor.GetData(), descriptor.GetDataSize());
                        } else {
                            fprintf(stderr, "ERROR: failed to create sample-aes descriptor (%d)\n", result);
                            return result;
                        }
                    }
                    if (video_track) {
                        result = MakeSampleAesVideoDescriptor(descriptor);
                        if (AP4_SUCCEEDED(result) && descriptor.GetDataSize()) {
                            video_stream->SetDescriptor(descriptor.GetData(), descriptor.GetDataSize());
                        } else {
                            fprintf(stderr, "ERROR: failed to create sample-aes descriptor (%d)\n", result);
                            return result;
                        }
                    }
                }
                
                ts_writer->WritePAT(*segment_output);
                ts_writer->WritePMT(*segment_output);
            } else if (packed_writer && chosen_track == audio_track) {
                AP4_DataBuffer       private_extension_buffer;
                const char*          private_extension_name = NULL;
                const unsigned char* private_extension_data = NULL;
                unsigned int         private_extension_data_size = 0;
                if (Options.encryption_mode == ENCRYPTION_MODE_SAMPLE_AES) {
                    private_extension_name = "com.apple.streaming.audioDescription";
                    result = MakeAudioSetupData(private_extension_buffer, audio_track->GetSampleDescription(0), audio_sample_data);
                    if (AP4_FAILED(result)) {
                        fprintf(stderr, "ERROR: failed to make audio setup data (%d)\n", result);
                        return 1;
                    }
                    private_extension_data      = private_extension_buffer.GetData();
                    private_extension_data_size = private_extension_buffer.GetDataSize();
                }
                packed_writer->WriteHeader(audio_ts,
                                           private_extension_name,
                                           private_extension_data,
                                           private_extension_data_size,
                                           *segment_output);
            }
        }

        // write the samples out and advance to the next sample
        if (chosen_track == audio_track) {
            // perform sample-level encryption if needed
            if (sample_encrypter) {
                result = sample_encrypter->EncryptAudioSample(audio_sample_data, audio_track->GetSampleDescription(audio_sample.GetDescriptionIndex()));
                if (AP4_FAILED(result)) {
                    fprintf(stderr, "ERROR: failed to encrypt audio sample (%d)\n", result);
                    return result;
                }
            }
            
            // write the sample data
            if (audio_stream) {
                result = audio_stream->WriteSample(audio_sample,
                                                   audio_sample_data,
                                                   audio_track->GetSampleDescription(audio_sample.GetDescriptionIndex()), 
                                                   video_track==NULL, 
                                                   *segment_output);
            } else if (packed_writer) {
                result = packed_writer->WriteSample(audio_sample,
                                                    audio_sample_data,
                                                    audio_track->GetSampleDescription(audio_sample.GetDescriptionIndex()),
                                                    *segment_output);
            } else {
                return AP4_ERROR_INTERNAL;
            }
            if (AP4_FAILED(result)) return result;
            
            result = ReadSample(*audio_reader, *audio_track, audio_sample, audio_sample_data, audio_ts, audio_frame_duration, audio_eos);
            if (AP4_FAILED(result)) return result;
            ++audio_sample_count;
        } else if (chosen_track == video_track) {
            // perform sample-level encryption if needed
            if (sample_encrypter) {
                result = sample_encrypter->EncryptVideoSample(video_sample_data, nalu_length_size);
                if (AP4_FAILED(result)) {
                    fprintf(stderr, "ERROR: failed to encrypt video sample (%d)\n", result);
                    return result;
                }
            }

            // write the sample data
            AP4_Position frame_start = 0;
            segment_output->Tell(frame_start);
            result = video_stream->WriteSample(video_sample,
                                               video_sample_data, 
                                               video_track->GetSampleDescription(video_sample.GetDescriptionIndex()),
                                               true, 
                                               *segment_output);
            if (AP4_FAILED(result)) return result;
            AP4_Position frame_end = 0;
            segment_output->Tell(frame_end);
            
            // measure I frames
            if (video_sample.IsSync()) {
                AP4_UI64 frame_size = 0;
                if (frame_end > frame_start) {
                    frame_size = frame_end-frame_start;
                }
                if (Options.encryption_mode == ENCRYPTION_MODE_AES_128) {
                    frame_start += segment_position;
                }
                iframe_positions.Append(frame_start);
                iframe_sizes.Append((AP4_UI32)frame_size);
                iframe_times.Append(video_ts);
                iframe_segment_indexes.Append(segment_number);
                iframe_durations.Append(0.0); // will be computed later
                if (Options.verbose) {
                    printf("I-Frame: %d@%lld, t=%f\n", (AP4_UI32)frame_size, frame_start, video_ts);
                }
            }
            
            // read the next sample
            result = ReadSample(*video_reader, *video_track, video_sample, video_sample_data, video_ts, video_frame_duration, video_eos);
            if (AP4_FAILED(result)) return result;
            ++video_sample_count;
        } else {
            break;
        }
    }
    
    // create the media playlist/index file
    playlist = OpenOutput(Options.index_filename, 0);
    if (playlist == NULL) return AP4_ERROR_CANNOT_OPEN_FILE;

    unsigned int target_duration = 0;
    double       total_duration = 0.0;
    for (unsigned int i=0; i<segment_durations.ItemCount(); i++) {
        if ((unsigned int)(segment_durations[i]+0.5) > target_duration) {
            target_duration = (unsigned int)segment_durations[i];
        }
        total_duration += segment_durations[i];
    }

    playlist->WriteString("#EXTM3U\r\n");
    if (Options.hls_version > 1) {
        sprintf(string_buffer, "#EXT-X-VERSION:%d\r\n", Options.hls_version);
        playlist->WriteString(string_buffer);
    }
    playlist->WriteString("#EXT-X-PLAYLIST-TYPE:VOD\r\n");
    if (video_track) {
        playlist->WriteString("#EXT-X-INDEPENDENT-SEGMENTS\r\n");
    }
    if (Options.allow_cache) {
        playlist->WriteString("#EXT-X-ALLOW-CACHE:");
        playlist->WriteString(Options.allow_cache);
        playlist->WriteString("\r\n");
    }
    playlist->WriteString("#EXT-X-TARGETDURATION:");
    sprintf(string_buffer, "%d\r\n", target_duration);
    playlist->WriteString(string_buffer);
    playlist->WriteString("#EXT-X-MEDIA-SEQUENCE:0\r\n");

    if (Options.encryption_mode != ENCRYPTION_MODE_NONE) {
        if (Options.encryption_key_lines.ItemCount()) {
            for (unsigned int i=0; i<Options.encryption_key_lines.ItemCount(); i++) {
                AP4_String& key_line = Options.encryption_key_lines[i];
                const char* key_line_cstr = key_line.GetChars();
                bool omit_iv = false;
                
                // omit the IV if the key line starts with a "!" (and skip the "!")
                if (key_line[0] == '!') {
                    ++key_line_cstr;
                    omit_iv = true;
                }
                
                playlist->WriteString("#EXT-X-KEY:METHOD=");
                if (Options.encryption_mode == ENCRYPTION_MODE_AES_128) {
                    playlist->WriteString("AES-128");
                } else if (Options.encryption_mode == ENCRYPTION_MODE_SAMPLE_AES) {
                    playlist->WriteString("SAMPLE-AES");
                }
                playlist->WriteString(",");
                playlist->WriteString(key_line_cstr);
                if ((Options.encryption_iv_mode == ENCRYPTION_IV_MODE_RANDOM ||
                     Options.encryption_iv_mode == ENCRYPTION_IV_MODE_FPS) && !omit_iv) {
                    playlist->WriteString(",IV=0x");
                    char iv_hex[33];
                    iv_hex[32] = 0;
                    AP4_FormatHex(Options.encryption_iv, 16, iv_hex);
                    playlist->WriteString(iv_hex);
                }
                playlist->WriteString("\r\n");
            }
        } else {
            playlist->WriteString("#EXT-X-KEY:METHOD=");
            if (Options.encryption_mode == ENCRYPTION_MODE_AES_128) {
                playlist->WriteString("AES-128");
            } else if (Options.encryption_mode == ENCRYPTION_MODE_SAMPLE_AES) {
                playlist->WriteString("SAMPLE-AES");
            }
            playlist->WriteString(",URI=\"");
            playlist->WriteString(Options.encryption_key_uri);
            playlist->WriteString("\"");
            if (Options.encryption_iv_mode == ENCRYPTION_IV_MODE_RANDOM) {
                playlist->WriteString(",IV=0x");
                char iv_hex[33];
                iv_hex[32] = 0;
                AP4_FormatHex(Options.encryption_iv, 16, iv_hex);
                playlist->WriteString(iv_hex);
            }
            if (Options.encryption_key_format) {
                playlist->WriteString(",KEYFORMAT=\"");
                playlist->WriteString(Options.encryption_key_format);
                playlist->WriteString("\"");
            }
            if (Options.encryption_key_format_versions) {
                playlist->WriteString(",KEYFORMATVERSIONS=\"");
                playlist->WriteString(Options.encryption_key_format_versions);
                playlist->WriteString("\"");
            }
            playlist->WriteString("\r\n");
        }
    }
    
    for (unsigned int i=0; i<segment_durations.ItemCount(); i++) {
        if (Options.hls_version >= 3) {
            sprintf(string_buffer, "#EXTINF:%f,\r\n", segment_durations[i]);
        } else {
            sprintf(string_buffer, "#EXTINF:%u,\r\n", (unsigned int)(segment_durations[i]+0.5));
        }
        playlist->WriteString(string_buffer);
        if (Options.output_single_file) {
            sprintf(string_buffer, "#EXT-X-BYTERANGE:%d@%lld\r\n", segment_sizes[i], segment_positions[i]);
            playlist->WriteString(string_buffer);
        }
        sprintf(string_buffer, Options.segment_url_template, i);
        playlist->WriteString(string_buffer);
        playlist->WriteString("\r\n");
    }
                    
    playlist->WriteString("#EXT-X-ENDLIST\r\n");
    playlist->Release();

    // create the iframe playlist/index file
    if (video_track && Options.hls_version >= 4) {
        // compute the iframe durations and target duration
        for (unsigned int i=0; i<iframe_positions.ItemCount(); i++) {
            double iframe_duration = 0.0;
            if (i+1 < iframe_positions.ItemCount()) {
                iframe_duration = iframe_times[i+1]-iframe_times[i];
            } else if (total_duration > iframe_times[i]) {
                iframe_duration = total_duration-iframe_times[i];
            }
            iframe_durations[i] = iframe_duration;
        }
        unsigned int iframes_target_duration = 0;
        for (unsigned int i=0; i<iframe_durations.ItemCount(); i++) {
            if ((unsigned int)(iframe_durations[i]+0.5) > iframes_target_duration) {
                iframes_target_duration = (unsigned int)iframe_durations[i];
            }
        }
        
        playlist = OpenOutput(Options.iframe_index_filename, 0);
        if (playlist == NULL) return AP4_ERROR_CANNOT_OPEN_FILE;

        playlist->WriteString("#EXTM3U\r\n");
        if (Options.hls_version > 1) {
            sprintf(string_buffer, "#EXT-X-VERSION:%d\r\n", Options.hls_version);
            playlist->WriteString(string_buffer);
        }
        playlist->WriteString("#EXT-X-PLAYLIST-TYPE:VOD\r\n");
        playlist->WriteString("#EXT-X-I-FRAMES-ONLY\r\n");
        playlist->WriteString("#EXT-X-INDEPENDENT-SEGMENTS\r\n");
        playlist->WriteString("#EXT-X-TARGETDURATION:");
        sprintf(string_buffer, "%d\r\n", iframes_target_duration);
        playlist->WriteString(string_buffer);
        playlist->WriteString("#EXT-X-MEDIA-SEQUENCE:0\r\n");

        if (Options.encryption_mode != ENCRYPTION_MODE_NONE) {
            playlist->WriteString("#EXT-X-KEY:METHOD=");
            if (Options.encryption_mode == ENCRYPTION_MODE_AES_128) {
                playlist->WriteString("AES-128");
            } else if (Options.encryption_mode == ENCRYPTION_MODE_SAMPLE_AES) {
                playlist->WriteString("SAMPLE-AES");
            }
            playlist->WriteString(",URI=\"");
            playlist->WriteString(Options.encryption_key_uri);
            playlist->WriteString("\"");
            if (Options.encryption_iv_mode == ENCRYPTION_IV_MODE_RANDOM) {
                playlist->WriteString(",IV=0x");
                char iv_hex[33];
                iv_hex[32] = 0;
                AP4_FormatHex(Options.encryption_iv, 16, iv_hex);
                playlist->WriteString(iv_hex);
            }
            if (Options.encryption_key_format) {
                playlist->WriteString(",KEYFORMAT=\"");
                playlist->WriteString(Options.encryption_key_format);
                playlist->WriteString("\"");
            }
            if (Options.encryption_key_format_versions) {
                playlist->WriteString(",KEYFORMATVERSIONS=\"");
                playlist->WriteString(Options.encryption_key_format_versions);
                playlist->WriteString("\"");
            }
            playlist->WriteString("\r\n");
        }
        
        for (unsigned int i=0; i<iframe_positions.ItemCount(); i++) {
            sprintf(string_buffer, "#EXTINF:%f,\r\n", iframe_durations[i]);
            playlist->WriteString(string_buffer);
            sprintf(string_buffer, "#EXT-X-BYTERANGE:%d@%lld\r\n", iframe_sizes[i], iframe_positions[i]);
            playlist->WriteString(string_buffer);
            sprintf(string_buffer, Options.segment_url_template, iframe_segment_indexes[i]);
            playlist->WriteString(string_buffer);
            playlist->WriteString("\r\n");
        }
                        
        playlist->WriteString("#EXT-X-ENDLIST\r\n");
        playlist->Release();
    }
    
    // update stats
    Stats.segment_count = segment_sizes.ItemCount();
    for (unsigned int i=0; i<segment_sizes.ItemCount(); i++) {
        Stats.segments_total_size     += segment_sizes[i];
        Stats.segments_total_duration += segment_durations[i];
    }
    Stats.iframe_count = iframe_sizes.ItemCount();
    for (unsigned int i=0; i<iframe_sizes.ItemCount(); i++) {
        Stats.iframes_total_size += iframe_sizes[i];
    }
    for (unsigned int i=0; i<iframe_positions.ItemCount(); i++) {
        if (iframe_durations[i] != 0.0) {
            double iframe_bitrate = 8.0*(double)iframe_sizes[i]/iframe_durations[i];
            if (iframe_bitrate > Stats.max_iframe_bitrate) {
                Stats.max_iframe_bitrate = iframe_bitrate;
            }
        }
    }
    
    if (Options.verbose) {
        printf("Conversion complete, total duration=%.2f secs\n", total_duration);
    }
    
    if (segment_output) segment_output->Release();
    delete sample_encrypter;
    
    return result;
}