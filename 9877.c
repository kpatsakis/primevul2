ReadSample(SampleReader&   reader, 
           AP4_Track&      track,
           AP4_Sample&     sample,
           AP4_DataBuffer& sample_data, 
           double&         ts,
           double&         duration,
           bool&           eos)
{
    AP4_Result result = reader.ReadSample(sample, sample_data);
    if (AP4_FAILED(result)) {
        if (result == AP4_ERROR_EOS) {
            // advance the timestamp by the last sample's duration
            ts += duration;
            eos = true;
            return AP4_SUCCESS;
        } else {
            return result;
        }
    }
    ts = (double)sample.GetDts()/(double)track.GetMediaTimeScale();
    duration = sample.GetDuration()/(double)track.GetMediaTimeScale();
    
    return AP4_SUCCESS;
}