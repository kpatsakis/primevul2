 explicit MyVorbisExtractor(const sp<DataSource> &source)
 : MyOggExtractor(source,
                MEDIA_MIMETYPE_AUDIO_VORBIS,
 /* numHeaders */ 3,
 /* seekPreRollUs */ 0) {
 }
