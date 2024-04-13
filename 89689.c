void MediaPlayerService::AudioOutput::setMinBufferCount()
{
 char value[PROPERTY_VALUE_MAX];
 if (property_get("ro.kernel.qemu", value, 0)) {
        mIsOnEmulator = true;
        mMinBufferCount = 12; // to prevent systematic buffer underrun for emulator
 }
}
