static size_t adev_get_input_buffer_size(const struct audio_hw_device *dev,
 const struct audio_config *config)
{
 (void)dev;

 /* NOTE: we default to built in mic which may cause a mismatch between what we
     * report here and the actual buffer size
     */
 return get_input_buffer_size(config->sample_rate,
                                 config->format,
                                 audio_channel_count_from_in_mask(config->channel_mask),
                                 PCM_CAPTURE /* usecase_type */,
                                 AUDIO_DEVICE_IN_BUILTIN_MIC);
}
