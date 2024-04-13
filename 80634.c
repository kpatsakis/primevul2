static bool CheckWebm(const uint8* buffer, int buffer_size) {
  RCHECK(buffer_size > 12);

  BitReader reader(buffer, buffer_size);

  RCHECK(GetElementId(&reader) == 0x1a45dfa3);

  int header_size = GetVint(&reader);
  RCHECK(reader.bits_available() / 8 >= header_size);

  while (reader.bits_available() > 0) {
    int tag = GetElementId(&reader);
    int tagsize = GetVint(&reader);
    switch (tag) {
      case 0x4286:  // EBMLVersion
      case 0x42f7:  // EBMLReadVersion
      case 0x42f2:  // EBMLMaxIdLength
      case 0x42f3:  // EBMLMaxSizeLength
      case 0x4287:  // DocTypeVersion
      case 0x4285:  // DocTypeReadVersion
      case 0xec:    // void
      case 0xbf:    // CRC32
        RCHECK(reader.SkipBits(tagsize * 8));
        break;

      case 0x4282:  // EBMLDocType
        switch (ReadBits(&reader, 32)) {
          case TAG('w', 'e', 'b', 'm') :
            return true;
          case TAG('m', 'a', 't', 'r') :
            return (ReadBits(&reader, 32) == TAG('o', 's', 'k', 'a'));
        }
        return false;

      default:  // Unrecognized tag
        return false;
    }
  }
  return false;
}
