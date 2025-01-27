static MagickOffsetType TIFFSeekCustomStream(const MagickOffsetType offset,
  const int whence,void *user_data)
{
  PhotoshopProfile
    *profile;

  profile=(PhotoshopProfile *) user_data;
  switch (whence)
  {
    case SEEK_SET:
    default:
    {
      if (offset < 0)
        return(-1);
      profile->offset=offset;
      break;
    }
    case SEEK_CUR:
    {
      if (((offset > 0) && (profile->offset > (MAGICK_SSIZE_MAX-offset))) ||
          ((offset < 0) && (profile->offset < (MAGICK_SSIZE_MIN-offset))))
        {
          errno=EOVERFLOW;
          return(-1);
        }
      if ((profile->offset+offset) < 0)
        return(-1);
      profile->offset+=offset;
      break;
    }
    case SEEK_END:
    {
      if (((MagickOffsetType) profile->length+offset) < 0)
        return(-1);
      profile->offset=profile->length+offset;
      break;
    }
  }

  return(profile->offset);
}