MakeSampleAesAudioDescriptor(AP4_DataBuffer&        descriptor,
                             AP4_SampleDescription* sample_description,
                             AP4_DataBuffer&        sample_data /* needed for encrypted AC3 */)
{
    // descriptor
    descriptor.SetDataSize(6);
    AP4_UI08* payload = descriptor.UseData();
    payload[0] = AP4_MPEG2_PRIVATE_DATA_INDICATOR_DESCRIPTOR_TAG;
    payload[1] = 4;
    if (sample_description->GetFormat() == AP4_SAMPLE_FORMAT_MP4A) {
        payload[2] = 'a';
        payload[3] = 'a';
        payload[4] = 'c';
        payload[5] = 'd';
    } else if (sample_description->GetFormat() == AP4_SAMPLE_FORMAT_AC_3) {
        payload[2] = 'a';
        payload[3] = 'c';
        payload[4] = '3';
        payload[5] = 'd';
    } else if (sample_description->GetFormat() == AP4_SAMPLE_FORMAT_EC_3) {
        payload[2] = 'e';
        payload[3] = 'c';
        payload[4] = '3';
        payload[5] = 'd';
    }

    // compute the audio setup data
    AP4_DataBuffer audio_setup_data;
    AP4_Result result = MakeAudioSetupData(audio_setup_data, sample_description, sample_data);
    if (AP4_FAILED(result)) {
        fprintf(stderr, "ERROR: failed to make audio setup data (%d)\n", result);
        return result;
    }
    
    descriptor.SetDataSize(descriptor.GetDataSize()+6+audio_setup_data.GetDataSize());
    payload = descriptor.UseData()+6;
    payload[0] = AP4_MPEG2_REGISTRATION_DESCRIPTOR_TAG;
    payload[1] = 4+audio_setup_data.GetDataSize();
    payload[2] = 'a';
    payload[3] = 'p';
    payload[4] = 'a';
    payload[5] = 'd';
    AP4_CopyMemory(&payload[6], audio_setup_data.GetData(), audio_setup_data.GetDataSize());
    
    return AP4_SUCCESS;
}