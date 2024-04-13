GBool DCTStream::readHeader() {
  GBool doScan;
  int n;
  int c = 0;
  int i;

  doScan = gFalse;
  while (!doScan) {
    c = readMarker();
    switch (c) {
    case 0xc0:			// SOF0 (sequential)
    case 0xc1:			// SOF1 (extended sequential)
      if (!readBaselineSOF()) {
	return gFalse;
      }
      break;
    case 0xc2:			// SOF2 (progressive)
      if (!readProgressiveSOF()) {
	return gFalse;
      }
      break;
    case 0xc4:			// DHT
      if (!readHuffmanTables()) {
	return gFalse;
      }
      break;
    case 0xd8:			// SOI
      break;
    case 0xd9:			// EOI
      return gFalse;
    case 0xda:			// SOS
      if (!readScanInfo()) {
	return gFalse;
      }
      doScan = gTrue;
      break;
    case 0xdb:			// DQT
      if (!readQuantTables()) {
	return gFalse;
      }
      break;
    case 0xdd:			// DRI
      if (!readRestartInterval()) {
	return gFalse;
      }
      break;
    case 0xe0:			// APP0
      if (!readJFIFMarker()) {
	return gFalse;
      }
      break;
    case 0xee:			// APP14
      if (!readAdobeMarker()) {
	return gFalse;
      }
      break;
    case EOF:
      error(errSyntaxError, getPos(), "Bad DCT header");
      return gFalse;
    default:
      if (c >= 0xe0) {
	n = read16() - 2;
	for (i = 0; i < n; ++i) {
	  str->getChar();
	}
      } else {
	error(errSyntaxError, getPos(), "Unknown DCT marker <{0:02x}>", c);
	return gFalse;
      }
      break;
    }
  }

  return gTrue;
}
