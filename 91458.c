void AMediaCodec_releaseName(
 AMediaCodec * /* mData */,
 char* name) {
 if (name != NULL) {
        free(name);
 }
}
