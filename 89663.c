bool MediaPlayerService::AudioOutput::isOnEmulator()
{
    setMinBufferCount(); // benign race wrt other threads
 return mIsOnEmulator;
}
