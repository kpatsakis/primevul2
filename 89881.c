void SoftVPX::onPortFlushCompleted(OMX_U32 portIndex) {
 if (portIndex == kInputPortIndex) {
 bool portWillReset = false;
 if (!outputBuffers(
 true /* flushDecoder */, false /* display */, false /* eos */, &portWillReset)) {
            ALOGE("Failed to flush decoder.");
            notify(OMX_EventError, OMX_ErrorUndefined, 0, NULL);
 return;
 }
        mEOSStatus = INPUT_DATA_AVAILABLE;
 }
}
