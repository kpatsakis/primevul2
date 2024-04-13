MakeAudioSetupData(AP4_DataBuffer&        audio_setup_data,
                   AP4_SampleDescription* sample_description,
                   AP4_DataBuffer&        sample_data /* needed for encrypted AC3 */)
{
    AP4_UI08       audio_type[4];
    AP4_DataBuffer setup_data;
    
    if (sample_description->GetFormat() == AP4_SAMPLE_FORMAT_MP4A) {
        AP4_MpegAudioSampleDescription* mpeg_audio_desc = AP4_DYNAMIC_CAST(AP4_MpegAudioSampleDescription, sample_description);
        if (mpeg_audio_desc == NULL ||
            !(mpeg_audio_desc->GetObjectTypeId() == AP4_OTI_MPEG4_AUDIO          ||
              mpeg_audio_desc->GetObjectTypeId() == AP4_OTI_MPEG2_AAC_AUDIO_LC   ||
              mpeg_audio_desc->GetObjectTypeId() == AP4_OTI_MPEG2_AAC_AUDIO_MAIN)) {
            return AP4_ERROR_NOT_SUPPORTED;
        }
        const AP4_DataBuffer& dsi = mpeg_audio_desc->GetDecoderInfo();
        setup_data.SetData(dsi.GetData(), dsi.GetDataSize());

        AP4_Mp4AudioDecoderConfig dec_config;
        AP4_Result result = dec_config.Parse(dsi.GetData(), dsi.GetDataSize());
        if (AP4_FAILED(result)) {
            return AP4_ERROR_INVALID_FORMAT;
        }

        if (dec_config.m_Extension.m_SbrPresent || dec_config.m_Extension.m_PsPresent) {
            if (dec_config.m_Extension.m_PsPresent) {
                audio_type[0] = 'z';
                audio_type[1] = 'a';
                audio_type[2] = 'c';
                audio_type[3] = 'p';
            } else {
                audio_type[0] = 'z';
                audio_type[1] = 'a';
                audio_type[2] = 'c';
                audio_type[3] = 'h';
            }
        } else {
            audio_type[0] = 'z';
            audio_type[1] = 'a';
            audio_type[2] = 'a';
            audio_type[3] = 'c';
        }
    } else if (sample_description->GetFormat() == AP4_SAMPLE_FORMAT_AC_3) {
        audio_type[0] = 'z';
        audio_type[1] = 'a';
        audio_type[2] = 'c';
        audio_type[3] = '3';

        // the setup data is the first 10 bytes of a syncframe
        if (sample_data.GetDataSize() >= 10) {
            setup_data.SetData(sample_data.GetData(), 10);
        }
    } else if (sample_description->GetFormat() == AP4_SAMPLE_FORMAT_EC_3) {
        audio_type[0] = 'z';
        audio_type[1] = 'e';
        audio_type[2] = 'c';
        audio_type[3] = '3';

        AP4_Dec3Atom* dec3 = AP4_DYNAMIC_CAST(AP4_Dec3Atom, sample_description->GetDetails().GetChild(AP4_ATOM_TYPE_DEC3));
        if (dec3 == NULL) {
            fprintf(stderr, "ERROR: failed to find 'dec3' atom in sample description\n");
            return 1;
        }
        setup_data.SetData(dec3->GetRawBytes().GetData(), dec3->GetRawBytes().GetDataSize());
    } else {
        return AP4_SUCCESS;
    }

    audio_setup_data.SetDataSize(8+setup_data.GetDataSize());
    AP4_UI08* payload = audio_setup_data.UseData();
    payload[0] = audio_type[0];
    payload[1] = audio_type[1];
    payload[2] = audio_type[2];
    payload[3] = audio_type[3];
    payload[4] = 0; // priming
    payload[5] = 0; // priming
    payload[6] = 1; // version
    payload[7] = setup_data.GetDataSize(); // setup_data_length
    if (setup_data.GetDataSize()) {
        AP4_CopyMemory(&payload[8], setup_data.GetData(), setup_data.GetDataSize());
    }

    return AP4_SUCCESS;
}