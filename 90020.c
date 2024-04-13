status_t AudioFlinger::EffectModule::configure()
{
 status_t status;
    sp<ThreadBase> thread;
 uint32_t size;
 audio_channel_mask_t channelMask;

 if (mEffectInterface == NULL) {
        status = NO_INIT;
 goto exit;
 }

    thread = mThread.promote();
 if (thread == 0) {
        status = DEAD_OBJECT;
 goto exit;
 }

    channelMask = thread->channelMask();
    mConfig.outputCfg.channels = channelMask;

 if ((mDescriptor.flags & EFFECT_FLAG_TYPE_MASK) == EFFECT_FLAG_TYPE_AUXILIARY) {
        mConfig.inputCfg.channels = AUDIO_CHANNEL_OUT_MONO;
 } else {
        mConfig.inputCfg.channels = channelMask;
 if (channelMask == AUDIO_CHANNEL_OUT_MONO) {
            mConfig.inputCfg.channels = AUDIO_CHANNEL_OUT_STEREO;
            mConfig.outputCfg.channels = AUDIO_CHANNEL_OUT_STEREO;
            ALOGV("Overriding effect input and output as STEREO");
 }
 }

    mConfig.inputCfg.format = AUDIO_FORMAT_PCM_16_BIT;
    mConfig.outputCfg.format = AUDIO_FORMAT_PCM_16_BIT;
    mConfig.inputCfg.samplingRate = thread->sampleRate();
    mConfig.outputCfg.samplingRate = mConfig.inputCfg.samplingRate;
    mConfig.inputCfg.bufferProvider.cookie = NULL;
    mConfig.inputCfg.bufferProvider.getBuffer = NULL;
    mConfig.inputCfg.bufferProvider.releaseBuffer = NULL;
    mConfig.outputCfg.bufferProvider.cookie = NULL;
    mConfig.outputCfg.bufferProvider.getBuffer = NULL;
    mConfig.outputCfg.bufferProvider.releaseBuffer = NULL;
    mConfig.inputCfg.accessMode = EFFECT_BUFFER_ACCESS_READ;
 if (mConfig.inputCfg.buffer.raw != mConfig.outputCfg.buffer.raw) {
        mConfig.outputCfg.accessMode = EFFECT_BUFFER_ACCESS_ACCUMULATE;
 } else {
        mConfig.outputCfg.accessMode = EFFECT_BUFFER_ACCESS_WRITE;
 }
    mConfig.inputCfg.mask = EFFECT_CONFIG_ALL;
    mConfig.outputCfg.mask = EFFECT_CONFIG_ALL;
    mConfig.inputCfg.buffer.frameCount = thread->frameCount();
    mConfig.outputCfg.buffer.frameCount = mConfig.inputCfg.buffer.frameCount;

    ALOGV("configure() %p thread %p buffer %p framecount %d",
 this, thread.get(), mConfig.inputCfg.buffer.raw, mConfig.inputCfg.buffer.frameCount);

 status_t cmdStatus;
    size = sizeof(int);
    status = (*mEffectInterface)->command(mEffectInterface,
                                                   EFFECT_CMD_SET_CONFIG,
 sizeof(effect_config_t),
 &mConfig,
 &size,
 &cmdStatus);
 if (status == 0) {
        status = cmdStatus;
 }

 if (status == 0 &&
 (memcmp(&mDescriptor.type, SL_IID_VISUALIZATION, sizeof(effect_uuid_t)) == 0)) {
 uint32_t buf32[sizeof(effect_param_t) / sizeof(uint32_t) + 2];
 effect_param_t *p = (effect_param_t *)buf32;

        p->psize = sizeof(uint32_t);
        p->vsize = sizeof(uint32_t);
        size = sizeof(int);
 *(int32_t *)p->data = VISUALIZER_PARAM_LATENCY;

 uint32_t latency = 0;
 PlaybackThread *pbt = thread->mAudioFlinger->checkPlaybackThread_l(thread->mId);
 if (pbt != NULL) {
            latency = pbt->latency_l();
 }

 *((int32_t *)p->data + 1)= latency;
 (*mEffectInterface)->command(mEffectInterface,
                                     EFFECT_CMD_SET_PARAM,
 sizeof(effect_param_t) + 8,
 &buf32,
 &size,
 &cmdStatus);
 }

    mMaxDisableWaitCnt = (MAX_DISABLE_TIME_MS * mConfig.outputCfg.samplingRate) /
 (1000 * mConfig.outputCfg.buffer.frameCount);

exit:
    mStatus = status;
 return status;
}
