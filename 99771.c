static MagickBooleanType PingGIFImage(Image *image)
{
  unsigned char
    buffer[256],
    length,
    data_size;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (ReadBlob(image,1,&data_size) != 1)
    ThrowBinaryImageException(CorruptImageError,"CorruptImage",
      image->filename);
  if (data_size > MaximumLZWBits)
    ThrowBinaryImageException(CorruptImageError,"CorruptImage",
      image->filename);
  if (ReadBlob(image,1,&length) != 1)
    ThrowBinaryImageException(CorruptImageError,"CorruptImage",
      image->filename);
  while (length != 0)
  {
    if (ReadBlob(image,length,buffer) != (ssize_t) length)
      ThrowBinaryImageException(CorruptImageError,"CorruptImage",
        image->filename);
    if (ReadBlob(image,1,&length) != 1)
      ThrowBinaryImageException(CorruptImageError,"CorruptImage",
        image->filename);
  }
  return(MagickTrue);
}
