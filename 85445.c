CString TextCodecUTF8::Encode(const UChar* characters,
                              wtf_size_t length,
                              UnencodableHandling) {
  return EncodeCommon(characters, length);
}
