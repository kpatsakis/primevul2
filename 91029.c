status_t OggSource::start(MetaData * /* params */) {
 if (mStarted) {
 return INVALID_OPERATION;
 }

    mStarted = true;

 return OK;
}
