SoftVPX::SoftVPX(
 const char *name,
 const char *componentRole,
        OMX_VIDEO_CODINGTYPE codingType,
 const OMX_CALLBACKTYPE *callbacks,
        OMX_PTR appData,
        OMX_COMPONENTTYPE **component)
 : SoftVideoDecoderOMXComponent(
            name, componentRole, codingType,
            NULL /* profileLevels */, 0 /* numProfileLevels */,
 320 /* width */, 240 /* height */, callbacks, appData, component),
      mMode(codingType == OMX_VIDEO_CodingVP8 ? MODE_VP8 : MODE_VP9),
      mCtx(NULL),
      mImg(NULL) {
 const size_t kMinCompressionRatio = mMode == MODE_VP8 ? 2 : 4;
 const char *mime = mMode == MODE_VP8 ? MEDIA_MIMETYPE_VIDEO_VP8 : MEDIA_MIMETYPE_VIDEO_VP9;
 const size_t kMaxOutputBufferSize = 2048 * 2048 * 3 / 2;
    initPorts(
            kNumBuffers, kMaxOutputBufferSize / kMinCompressionRatio /* inputBufferSize */,
            kNumBuffers, mime, kMinCompressionRatio);
    CHECK_EQ(initDecoder(), (status_t)OK);
}
