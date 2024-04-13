 virtual status_t readNextPacket(MediaBuffer **buffer) {
 return _readNextPacket(buffer, /* calcVorbisTimestamp = */ true);
 }
