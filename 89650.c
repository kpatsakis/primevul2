int MediaPlayerService::AudioOutput::getMinBufferCount()
{
    setMinBufferCount(); // benign race wrt other threads
 return mMinBufferCount;
}
