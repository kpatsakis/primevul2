void SoftVPX::onReset() {
 bool portWillReset = false;
 if (!outputBuffers(
 true /* flushDecoder */, false /* display */, false /* eos */, &portWillReset)) {
        ALOGW("Failed to flush decoder. Try to hard reset decoder");
        destroyDecoder();
        initDecoder();
 }
    mEOSStatus = INPUT_DATA_AVAILABLE;
}
