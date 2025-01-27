static inline unsigned short ReadProfileShort(const EndianType endian,
  unsigned char *buffer)
{
  unsigned short
    value;

  if (endian == LSBEndian)
    {
      value=(unsigned short) ((buffer[1] << 8) | buffer[0]);
      return((unsigned short) (value & 0xffff));
    }
  value=(unsigned short) ((((unsigned char *) buffer)[0] << 8) |
    ((unsigned char *) buffer)[1]);
  return((unsigned short) (value & 0xffff));
}
