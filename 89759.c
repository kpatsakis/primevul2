static size_t getFrameSize(
        OMX_COLOR_FORMATTYPE colorFormat, int32_t width, int32_t height) {
 switch (colorFormat) {
 case OMX_COLOR_FormatYCbYCr:
 case OMX_COLOR_FormatCbYCrY:
 return width * height * 2;

 case OMX_COLOR_FormatYUV420Planar:
 case OMX_COLOR_FormatYUV420SemiPlanar:
 case OMX_TI_COLOR_FormatYUV420PackedSemiPlanar:
 /*
        * FIXME: For the Opaque color format, the frame size does not
        * need to be (w*h*3)/2. It just needs to
        * be larger than certain minimum buffer size. However,
        * currently, this opaque foramt has been tested only on
        * YUV420 formats. If that is changed, then we need to revisit
        * this part in the future
        */
 case OMX_COLOR_FormatAndroidOpaque:
 return (width * height * 3) / 2;

 default:
            CHECK(!"Should not be here. Unsupported color format.");
 break;
 }
 return 0;
}
