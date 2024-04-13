void OMXCodec::onEvent(OMX_EVENTTYPE event, OMX_U32 data1, OMX_U32 data2) {
 switch (event) {
 case OMX_EventCmdComplete:
 {
            onCmdComplete((OMX_COMMANDTYPE)data1, data2);
 break;
 }

 case OMX_EventError:
 {
            CODEC_LOGE("OMX_EventError(0x%08x, %u)", data1, data2);

            setState(ERROR);
 break;
 }

 case OMX_EventPortSettingsChanged:
 {
            CODEC_LOGV("OMX_EventPortSettingsChanged(port=%u, data2=0x%08x)",
                       data1, data2);

 if (data2 == 0 || data2 == OMX_IndexParamPortDefinition) {
                onPortSettingsChanged(data1);
 } else if (data1 == kPortIndexOutput &&
 (data2 == OMX_IndexConfigCommonOutputCrop ||
                         data2 == OMX_IndexConfigCommonScale)) {

                sp<MetaData> oldOutputFormat = mOutputFormat;
                initOutputFormat(mSource->getFormat());

 if (data2 == OMX_IndexConfigCommonOutputCrop &&
                    formatHasNotablyChanged(oldOutputFormat, mOutputFormat)) {
                    mOutputPortSettingsHaveChanged = true;

 } else if (data2 == OMX_IndexConfigCommonScale) {
                    OMX_CONFIG_SCALEFACTORTYPE scale;
 InitOMXParams(&scale);
                    scale.nPortIndex = kPortIndexOutput;

 if (OK == mOMX->getConfig(
                                        mNode,
                                        OMX_IndexConfigCommonScale,
 &scale, sizeof(scale))) {
 int32_t left, top, right, bottom;
                        CHECK(mOutputFormat->findRect(kKeyCropRect,
 &left, &top,
 &right, &bottom));

                        ALOGV("Get OMX_IndexConfigScale: 0x%x/0x%x",
                                scale.xWidth, scale.xHeight);

 if (scale.xWidth != 0x010000) {
                            mOutputFormat->setInt32(kKeyDisplayWidth,
 ((right - left + 1) * scale.xWidth) >> 16);
                            mOutputPortSettingsHaveChanged = true;
 }

 if (scale.xHeight != 0x010000) {
                            mOutputFormat->setInt32(kKeyDisplayHeight,
 ((bottom  - top + 1) * scale.xHeight) >> 16);
                            mOutputPortSettingsHaveChanged = true;
 }
 }
 }
 }
 break;
 }

#if 0
 case OMX_EventBufferFlag:
 {
            CODEC_LOGV("EVENT_BUFFER_FLAG(%ld)", data1);

 if (data1 == kPortIndexOutput) {
                mNoMoreOutputData = true;
 }
 break;
 }
#endif

 default:
 {
            CODEC_LOGV("EVENT(%d, %u, %u)", event, data1, data2);
 break;
 }
 }
}
