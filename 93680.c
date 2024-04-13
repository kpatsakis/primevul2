static Image *ReadBMPImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  BMPInfo
    bmp_info;

  Image
    *image;

  MagickBooleanType
    status;

  MagickOffsetType
    offset,
    start_position;

  MemoryInfo
    *pixel_info;

  Quantum
    index;

  register Quantum
    *q;

  register ssize_t
    i,
    x;

  register unsigned char
    *p;

  size_t
    bit,
    bytes_per_line,
    length;

  ssize_t
    count,
    y;

  unsigned char
    magick[12],
    *pixels;

  unsigned int
    blue,
    green,
    offset_bits,
    red;

  /*
    Open image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  image=AcquireImage(image_info,exception);
  status=OpenBlob(image_info,image,ReadBinaryBlobMode,exception);
  if (status == MagickFalse)
    {
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  /*
    Determine if this a BMP file.
  */
  (void) memset(&bmp_info,0,sizeof(bmp_info));
  bmp_info.ba_offset=0;
  start_position=0;
  offset_bits=0;
  count=ReadBlob(image,2,magick);
  if (count != 2)
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  do
  {
    PixelInfo
      quantum_bits;

    PixelPacket
      shift;

    /*
      Verify BMP identifier.
    */
    if (bmp_info.ba_offset == 0)
      start_position=TellBlob(image)-2;
    bmp_info.ba_offset=0;
    while (LocaleNCompare((char *) magick,"BA",2) == 0)
    {
      bmp_info.file_size=ReadBlobLSBLong(image);
      bmp_info.ba_offset=ReadBlobLSBLong(image);
      bmp_info.offset_bits=ReadBlobLSBLong(image);
      count=ReadBlob(image,2,magick);
      if (count != 2)
        break;
    }
    if (image->debug != MagickFalse)
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),"  Magick: %c%c",
        magick[0],magick[1]);
    if ((count != 2) || ((LocaleNCompare((char *) magick,"BM",2) != 0) &&
        (LocaleNCompare((char *) magick,"CI",2) != 0)))
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
    bmp_info.file_size=ReadBlobLSBLong(image);
    (void) ReadBlobLSBLong(image);
    bmp_info.offset_bits=ReadBlobLSBLong(image);
    bmp_info.size=ReadBlobLSBLong(image);
    if (image->debug != MagickFalse)
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),"  BMP size: %u",
        bmp_info.size);
    if (bmp_info.size == 12)
      {
        /*
          OS/2 BMP image file.
        */
        (void) CopyMagickString(image->magick,"BMP2",MagickPathExtent);
        bmp_info.width=(ssize_t) ((short) ReadBlobLSBShort(image));
        bmp_info.height=(ssize_t) ((short) ReadBlobLSBShort(image));
        bmp_info.planes=ReadBlobLSBShort(image);
        bmp_info.bits_per_pixel=ReadBlobLSBShort(image);
        bmp_info.x_pixels=0;
        bmp_info.y_pixels=0;
        bmp_info.number_colors=0;
        bmp_info.compression=BI_RGB;
        bmp_info.image_size=0;
        bmp_info.alpha_mask=0;
        if (image->debug != MagickFalse)
          {
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "  Format: OS/2 Bitmap");
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "  Geometry: %.20gx%.20g",(double) bmp_info.width,(double)
              bmp_info.height);
          }
      }
    else
      {
        /*
          Microsoft Windows BMP image file.
        */
        if (bmp_info.size < 40)
          ThrowReaderException(CorruptImageError,"NonOS2HeaderSizeError");
        bmp_info.width=(ssize_t) ReadBlobLSBSignedLong(image);
        bmp_info.height=(ssize_t) ReadBlobLSBSignedLong(image);
        bmp_info.planes=ReadBlobLSBShort(image);
        bmp_info.bits_per_pixel=ReadBlobLSBShort(image);
        bmp_info.compression=ReadBlobLSBLong(image);
        bmp_info.image_size=ReadBlobLSBLong(image);
         bmp_info.x_pixels=ReadBlobLSBLong(image);
         bmp_info.y_pixels=ReadBlobLSBLong(image);
         bmp_info.number_colors=ReadBlobLSBLong(image);
        if (bmp_info.number_colors > GetBlobSize(image))
          ThrowReaderException(CorruptImageError,"InsufficientImageDataInFile");
         bmp_info.colors_important=ReadBlobLSBLong(image);
         if (image->debug != MagickFalse)
           {
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "  Format: MS Windows bitmap");
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "  Geometry: %.20gx%.20g",(double) bmp_info.width,(double)
              bmp_info.height);
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "  Bits per pixel: %.20g",(double) bmp_info.bits_per_pixel);
            switch (bmp_info.compression)
            {
              case BI_RGB:
              {
                (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                  "  Compression: BI_RGB");
                break;
              }
              case BI_RLE4:
              {
                (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                  "  Compression: BI_RLE4");
                break;
              }
              case BI_RLE8:
              {
                (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                  "  Compression: BI_RLE8");
                break;
              }
              case BI_BITFIELDS:
              {
                (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                  "  Compression: BI_BITFIELDS");
                break;
              }
              case BI_PNG:
              {
                (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                  "  Compression: BI_PNG");
                break;
              }
              case BI_JPEG:
              {
                (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                  "  Compression: BI_JPEG");
                break;
              }
              default:
              {
                (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                  "  Compression: UNKNOWN (%u)",bmp_info.compression);
              }
            }
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "  Number of colors: %u",bmp_info.number_colors);
          }
        bmp_info.red_mask=ReadBlobLSBLong(image);
        bmp_info.green_mask=ReadBlobLSBLong(image);
        bmp_info.blue_mask=ReadBlobLSBLong(image);
        if (bmp_info.size > 40)
          {
            double
              gamma;

            /*
              Read color management information.
            */
            bmp_info.alpha_mask=ReadBlobLSBLong(image);
            bmp_info.colorspace=ReadBlobLSBSignedLong(image);
            /*
              Decode 2^30 fixed point formatted CIE primaries.
            */
#           define BMP_DENOM ((double) 0x40000000)
            bmp_info.red_primary.x=(double) ReadBlobLSBLong(image)/BMP_DENOM;
            bmp_info.red_primary.y=(double) ReadBlobLSBLong(image)/BMP_DENOM;
            bmp_info.red_primary.z=(double) ReadBlobLSBLong(image)/BMP_DENOM;
            bmp_info.green_primary.x=(double) ReadBlobLSBLong(image)/BMP_DENOM;
            bmp_info.green_primary.y=(double) ReadBlobLSBLong(image)/BMP_DENOM;
            bmp_info.green_primary.z=(double) ReadBlobLSBLong(image)/BMP_DENOM;
            bmp_info.blue_primary.x=(double) ReadBlobLSBLong(image)/BMP_DENOM;
            bmp_info.blue_primary.y=(double) ReadBlobLSBLong(image)/BMP_DENOM;
            bmp_info.blue_primary.z=(double) ReadBlobLSBLong(image)/BMP_DENOM;

            gamma=bmp_info.red_primary.x+bmp_info.red_primary.y+
              bmp_info.red_primary.z;
            gamma=PerceptibleReciprocal(gamma);
            bmp_info.red_primary.x*=gamma;
            bmp_info.red_primary.y*=gamma;
            image->chromaticity.red_primary.x=bmp_info.red_primary.x;
            image->chromaticity.red_primary.y=bmp_info.red_primary.y;

            gamma=bmp_info.green_primary.x+bmp_info.green_primary.y+
              bmp_info.green_primary.z;
            gamma=PerceptibleReciprocal(gamma);
            bmp_info.green_primary.x*=gamma;
            bmp_info.green_primary.y*=gamma;
            image->chromaticity.green_primary.x=bmp_info.green_primary.x;
            image->chromaticity.green_primary.y=bmp_info.green_primary.y;

            gamma=bmp_info.blue_primary.x+bmp_info.blue_primary.y+
              bmp_info.blue_primary.z;
            gamma=PerceptibleReciprocal(gamma);
            bmp_info.blue_primary.x*=gamma;
            bmp_info.blue_primary.y*=gamma;
            image->chromaticity.blue_primary.x=bmp_info.blue_primary.x;
            image->chromaticity.blue_primary.y=bmp_info.blue_primary.y;

            /*
              Decode 16^16 fixed point formatted gamma_scales.
            */
            bmp_info.gamma_scale.x=(double) ReadBlobLSBLong(image)/0x10000;
            bmp_info.gamma_scale.y=(double) ReadBlobLSBLong(image)/0x10000;
            bmp_info.gamma_scale.z=(double) ReadBlobLSBLong(image)/0x10000;
            /*
              Compute a single gamma from the BMP 3-channel gamma.
            */
            image->gamma=(bmp_info.gamma_scale.x+bmp_info.gamma_scale.y+
              bmp_info.gamma_scale.z)/3.0;
          }
        else
          (void) CopyMagickString(image->magick,"BMP3",MagickPathExtent);

        if (bmp_info.size > 108)
          {
            size_t
              intent;

            /*
              Read BMP Version 5 color management information.
            */
            intent=ReadBlobLSBLong(image);
            switch ((int) intent)
            {
              case LCS_GM_BUSINESS:
              {
                image->rendering_intent=SaturationIntent;
                break;
              }
              case LCS_GM_GRAPHICS:
              {
                image->rendering_intent=RelativeIntent;
                break;
              }
              case LCS_GM_IMAGES:
              {
                image->rendering_intent=PerceptualIntent;
                break;
              }
              case LCS_GM_ABS_COLORIMETRIC:
              {
                image->rendering_intent=AbsoluteIntent;
                break;
              }
            }
            (void) ReadBlobLSBLong(image);  /* Profile data */
            (void) ReadBlobLSBLong(image);  /* Profile size */
            (void) ReadBlobLSBLong(image);  /* Reserved byte */
          }
      }
    if ((MagickSizeType) bmp_info.file_size > GetBlobSize(image))
      (void) ThrowMagickException(exception,GetMagickModule(),CorruptImageError,
        "LengthAndFilesizeDoNotMatch","`%s'",image->filename);
    else
      if ((MagickSizeType) bmp_info.file_size < GetBlobSize(image))
        (void) ThrowMagickException(exception,GetMagickModule(),
          CorruptImageWarning,"LengthAndFilesizeDoNotMatch","`%s'",
          image->filename);
    if (bmp_info.width <= 0)
      ThrowReaderException(CorruptImageError,"NegativeOrZeroImageSize");
    if (bmp_info.height == 0)
      ThrowReaderException(CorruptImageError,"NegativeOrZeroImageSize");
    if (bmp_info.planes != 1)
      ThrowReaderException(CorruptImageError,"StaticPlanesValueNotEqualToOne");
    if ((bmp_info.bits_per_pixel != 1) && (bmp_info.bits_per_pixel != 4) &&
        (bmp_info.bits_per_pixel != 8) && (bmp_info.bits_per_pixel != 16) &&
        (bmp_info.bits_per_pixel != 24) && (bmp_info.bits_per_pixel != 32))
      ThrowReaderException(CorruptImageError,"UnrecognizedBitsPerPixel");
    if (bmp_info.bits_per_pixel < 16 &&
        bmp_info.number_colors > (1U << bmp_info.bits_per_pixel))
      ThrowReaderException(CorruptImageError,"UnrecognizedNumberOfColors");
    if ((bmp_info.compression == 1) && (bmp_info.bits_per_pixel != 8))
      ThrowReaderException(CorruptImageError,"UnrecognizedBitsPerPixel");
    if ((bmp_info.compression == 2) && (bmp_info.bits_per_pixel != 4))
      ThrowReaderException(CorruptImageError,"UnrecognizedBitsPerPixel");
    if ((bmp_info.compression == 3) && (bmp_info.bits_per_pixel < 16))
      ThrowReaderException(CorruptImageError,"UnrecognizedBitsPerPixel");
    switch (bmp_info.compression)
    {
      case BI_RGB:
        image->compression=NoCompression;
        break;
      case BI_RLE8:
      case BI_RLE4:
        image->compression=RLECompression;
        break;
      case BI_BITFIELDS:
        break;
      case BI_JPEG:
        ThrowReaderException(CoderError,"JPEGCompressNotSupported");
      case BI_PNG:
        ThrowReaderException(CoderError,"PNGCompressNotSupported");
      default:
        ThrowReaderException(CorruptImageError,"UnrecognizedImageCompression");
    }
    image->columns=(size_t) MagickAbsoluteValue(bmp_info.width);
    image->rows=(size_t) MagickAbsoluteValue(bmp_info.height);
    image->depth=bmp_info.bits_per_pixel <= 8 ? bmp_info.bits_per_pixel : 8;
    image->alpha_trait=((bmp_info.alpha_mask != 0) &&
      (bmp_info.compression == BI_BITFIELDS)) ? BlendPixelTrait :
      UndefinedPixelTrait;
    if (bmp_info.bits_per_pixel < 16)
      {
        size_t
          one;

        image->storage_class=PseudoClass;
        image->colors=bmp_info.number_colors;
        one=1;
        if (image->colors == 0)
          image->colors=one << bmp_info.bits_per_pixel;
      }
    image->resolution.x=(double) bmp_info.x_pixels/100.0;
    image->resolution.y=(double) bmp_info.y_pixels/100.0;
    image->units=PixelsPerCentimeterResolution;
    if ((image_info->ping != MagickFalse) && (image_info->number_scenes != 0))
      if (image->scene >= (image_info->scene+image_info->number_scenes-1))
        break;
    status=SetImageExtent(image,image->columns,image->rows,exception);
    if (status == MagickFalse)
      return(DestroyImageList(image));
    if (image->storage_class == PseudoClass)
      {
        unsigned char
          *bmp_colormap;

        size_t
          packet_size;

        /*
          Read BMP raster colormap.
        */
        if (image->debug != MagickFalse)
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "  Reading colormap of %.20g colors",(double) image->colors);
        if (AcquireImageColormap(image,image->colors,exception) == MagickFalse)
          ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
        bmp_colormap=(unsigned char *) AcquireQuantumMemory((size_t)
          image->colors,4*sizeof(*bmp_colormap));
        if (bmp_colormap == (unsigned char *) NULL)
          ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
        if ((bmp_info.size == 12) || (bmp_info.size == 64))
          packet_size=3;
        else
          packet_size=4;
        offset=SeekBlob(image,start_position+14+bmp_info.size,SEEK_SET);
        if (offset < 0)
          {
            bmp_colormap=(unsigned char *) RelinquishMagickMemory(bmp_colormap);
            ThrowReaderException(CorruptImageError,"ImproperImageHeader");
          }
        count=ReadBlob(image,packet_size*image->colors,bmp_colormap);
        if (count != (ssize_t) (packet_size*image->colors))
          {
            bmp_colormap=(unsigned char *) RelinquishMagickMemory(bmp_colormap);
            ThrowReaderException(CorruptImageError,
              "InsufficientImageDataInFile");
          }
        p=bmp_colormap;
        for (i=0; i < (ssize_t) image->colors; i++)
        {
          image->colormap[i].blue=(MagickRealType) ScaleCharToQuantum(*p++);
          image->colormap[i].green=(MagickRealType) ScaleCharToQuantum(*p++);
          image->colormap[i].red=(MagickRealType) ScaleCharToQuantum(*p++);
          if (packet_size == 4)
            p++;
        }
        bmp_colormap=(unsigned char *) RelinquishMagickMemory(bmp_colormap);
      }
    /*
      Read image data.
    */
    if (bmp_info.offset_bits == offset_bits)
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
    offset_bits=bmp_info.offset_bits;
    offset=SeekBlob(image,start_position+bmp_info.offset_bits,SEEK_SET);
    if (offset < 0)
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
    if (bmp_info.compression == BI_RLE4)
      bmp_info.bits_per_pixel<<=1;
    bytes_per_line=4*((image->columns*bmp_info.bits_per_pixel+31)/32);
    length=(size_t) bytes_per_line*image->rows;
    if (((MagickSizeType) length/8) > GetBlobSize(image))
      ThrowReaderException(CorruptImageError,"InsufficientImageDataInFile");
    if ((bmp_info.compression == BI_RGB) ||
        (bmp_info.compression == BI_BITFIELDS))
      {
        pixel_info=AcquireVirtualMemory(image->rows,
          MagickMax(bytes_per_line,image->columns+256UL)*sizeof(*pixels));
        if (pixel_info == (MemoryInfo *) NULL)
          ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
        pixels=(unsigned char *) GetVirtualMemoryBlob(pixel_info);
        if (image->debug != MagickFalse)
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "  Reading pixels (%.20g bytes)",(double) length);
        count=ReadBlob(image,length,pixels);
        if (count != (ssize_t) length)
          {
            pixel_info=RelinquishVirtualMemory(pixel_info);
            ThrowReaderException(CorruptImageError,
              "InsufficientImageDataInFile");
          }
      }
    else
      {
        /*
          Convert run-length encoded raster pixels.
        */
        pixel_info=AcquireVirtualMemory(image->rows,
          MagickMax(bytes_per_line,image->columns+256UL)*sizeof(*pixels));
        if (pixel_info == (MemoryInfo *) NULL)
          ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
        pixels=(unsigned char *) GetVirtualMemoryBlob(pixel_info);
        status=DecodeImage(image,bmp_info.compression,pixels,
          image->columns*image->rows);
        if (status == MagickFalse)
          {
            pixel_info=RelinquishVirtualMemory(pixel_info);
            ThrowReaderException(CorruptImageError,
              "UnableToRunlengthDecodeImage");
          }
      }
    /*
      Convert BMP raster image to pixel packets.
    */
    if (bmp_info.compression == BI_RGB)
      {
        /*
          We should ignore the alpha value in BMP3 files but there have been
          reports about 32 bit files with alpha. We do a quick check to see if
          the alpha channel contains a value that is not zero (default value).
          If we find a non zero value we asume the program that wrote the file
          wants to use the alpha channel.
        */
        if ((image->alpha_trait == UndefinedPixelTrait) && (bmp_info.size == 40) &&
            (bmp_info.bits_per_pixel == 32))
          {
            bytes_per_line=4*(image->columns);
            for (y=(ssize_t) image->rows-1; y >= 0; y--)
            {
              p=pixels+(image->rows-y-1)*bytes_per_line;
              for (x=0; x < (ssize_t) image->columns; x++)
              {
                if (*(p+3) != 0)
                  {
                    image->alpha_trait=BlendPixelTrait;
                    y=-1;
                    break;
                  }
                p+=4;
              }
            }
          }
        bmp_info.alpha_mask=image->alpha_trait != UndefinedPixelTrait ?
          0xff000000U : 0U;
        bmp_info.red_mask=0x00ff0000U;
        bmp_info.green_mask=0x0000ff00U;
        bmp_info.blue_mask=0x000000ffU;
        if (bmp_info.bits_per_pixel == 16)
          {
            /*
              RGB555.
            */
            bmp_info.red_mask=0x00007c00U;
            bmp_info.green_mask=0x000003e0U;
            bmp_info.blue_mask=0x0000001fU;
          }
      }
    (void) memset(&shift,0,sizeof(shift));
    (void) memset(&quantum_bits,0,sizeof(quantum_bits));
    if ((bmp_info.bits_per_pixel == 16) || (bmp_info.bits_per_pixel == 32))
      {
        register unsigned int
          sample;

        /*
          Get shift and quantum bits info from bitfield masks.
        */
        if (bmp_info.red_mask != 0)
          while (((bmp_info.red_mask << shift.red) & 0x80000000UL) == 0)
          {
            shift.red++;
            if (shift.red >= 32U)
              break;
          }
        if (bmp_info.green_mask != 0)
          while (((bmp_info.green_mask << shift.green) & 0x80000000UL) == 0)
          {
            shift.green++;
            if (shift.green >= 32U)
              break;
          }
        if (bmp_info.blue_mask != 0)
          while (((bmp_info.blue_mask << shift.blue) & 0x80000000UL) == 0)
          {
            shift.blue++;
            if (shift.blue >= 32U)
              break;
          }
        if (bmp_info.alpha_mask != 0)
          while (((bmp_info.alpha_mask << shift.alpha) & 0x80000000UL) == 0)
          {
            shift.alpha++;
            if (shift.alpha >= 32U)
              break;
          }
        sample=shift.red;
        while (((bmp_info.red_mask << sample) & 0x80000000UL) != 0)
        {
          sample++;
          if (sample >= 32U)
            break;
        }
        quantum_bits.red=(MagickRealType) (sample-shift.red);
        sample=shift.green;
        while (((bmp_info.green_mask << sample) & 0x80000000UL) != 0)
        {
          sample++;
          if (sample >= 32U)
            break;
        }
        quantum_bits.green=(MagickRealType) (sample-shift.green);
        sample=shift.blue;
        while (((bmp_info.blue_mask << sample) & 0x80000000UL) != 0)
        {
          sample++;
          if (sample >= 32U)
            break;
        }
        quantum_bits.blue=(MagickRealType) (sample-shift.blue);
        sample=shift.alpha;
        while (((bmp_info.alpha_mask << sample) & 0x80000000UL) != 0)
        {
          sample++;
          if (sample >= 32U)
            break;
        }
        quantum_bits.alpha=(MagickRealType) (sample-shift.alpha);
      }
    switch (bmp_info.bits_per_pixel)
    {
      case 1:
      {
        /*
          Convert bitmap scanline.
        */
        for (y=(ssize_t) image->rows-1; y >= 0; y--)
        {
          p=pixels+(image->rows-y-1)*bytes_per_line;
          q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
          if (q == (Quantum *) NULL)
            break;
          for (x=0; x < ((ssize_t) image->columns-7); x+=8)
          {
            for (bit=0; bit < 8; bit++)
            {
              index=(Quantum) (((*p) & (0x80 >> bit)) != 0 ? 0x01 : 0x00);
              SetPixelIndex(image,index,q);
              q+=GetPixelChannels(image);
            }
            p++;
          }
          if ((image->columns % 8) != 0)
            {
              for (bit=0; bit < (image->columns % 8); bit++)
              {
                index=(Quantum) (((*p) & (0x80 >> bit)) != 0 ? 0x01 : 0x00);
                SetPixelIndex(image,index,q);
                q+=GetPixelChannels(image);
              }
              p++;
            }
          if (SyncAuthenticPixels(image,exception) == MagickFalse)
            break;
          if (image->previous == (Image *) NULL)
            {
              status=SetImageProgress(image,LoadImageTag,(MagickOffsetType)
                (image->rows-y),image->rows);
              if (status == MagickFalse)
                break;
            }
        }
        (void) SyncImage(image,exception);
        break;
      }
      case 4:
      {
        /*
          Convert PseudoColor scanline.
        */
        for (y=(ssize_t) image->rows-1; y >= 0; y--)
        {
          p=pixels+(image->rows-y-1)*bytes_per_line;
          q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
          if (q == (Quantum *) NULL)
            break;
          for (x=0; x < ((ssize_t) image->columns-1); x+=2)
          {
            ValidateColormapValue(image,(ssize_t) ((*p >> 4) & 0x0f),&index,
              exception);
            SetPixelIndex(image,index,q);
            q+=GetPixelChannels(image);
            ValidateColormapValue(image,(ssize_t) (*p & 0x0f),&index,exception);
            SetPixelIndex(image,index,q);
            q+=GetPixelChannels(image);
            p++;
          }
          if ((image->columns % 2) != 0)
            {
              ValidateColormapValue(image,(ssize_t) ((*p >> 4) & 0xf),&index,
                exception);
              SetPixelIndex(image,index,q);
              q+=GetPixelChannels(image);
              p++;
              x++;
            }
          if (x < (ssize_t) image->columns)
            break;
          if (SyncAuthenticPixels(image,exception) == MagickFalse)
            break;
          if (image->previous == (Image *) NULL)
            {
              status=SetImageProgress(image,LoadImageTag,(MagickOffsetType)
                (image->rows-y),image->rows);
              if (status == MagickFalse)
                break;
            }
        }
        (void) SyncImage(image,exception);
        break;
      }
      case 8:
      {
        /*
          Convert PseudoColor scanline.
        */
        if ((bmp_info.compression == BI_RLE8) ||
            (bmp_info.compression == BI_RLE4))
          bytes_per_line=image->columns;
        for (y=(ssize_t) image->rows-1; y >= 0; y--)
        {
          p=pixels+(image->rows-y-1)*bytes_per_line;
          q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
          if (q == (Quantum *) NULL)
            break;
          for (x=(ssize_t) image->columns; x != 0; --x)
          {
            ValidateColormapValue(image,(ssize_t) *p++,&index,exception);
            SetPixelIndex(image,index,q);
            q+=GetPixelChannels(image);
          }
          if (SyncAuthenticPixels(image,exception) == MagickFalse)
            break;
          offset=(MagickOffsetType) (image->rows-y-1);
          if (image->previous == (Image *) NULL)
            {
              status=SetImageProgress(image,LoadImageTag,(MagickOffsetType)
                (image->rows-y),image->rows);
              if (status == MagickFalse)
                break;
            }
        }
        (void) SyncImage(image,exception);
        break;
      }
      case 16:
      {
        unsigned int
          alpha,
          pixel;

        /*
          Convert bitfield encoded 16-bit PseudoColor scanline.
        */
        if ((bmp_info.compression != BI_RGB) &&
            (bmp_info.compression != BI_BITFIELDS))
          {
            pixel_info=RelinquishVirtualMemory(pixel_info);
            ThrowReaderException(CorruptImageError,
              "UnrecognizedImageCompression");
          }
        bytes_per_line=2*(image->columns+image->columns % 2);
        image->storage_class=DirectClass;
        for (y=(ssize_t) image->rows-1; y >= 0; y--)
        {
          p=pixels+(image->rows-y-1)*bytes_per_line;
          q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
          if (q == (Quantum *) NULL)
            break;
          for (x=0; x < (ssize_t) image->columns; x++)
          {
            pixel=(unsigned int) (*p++);
            pixel|=(*p++) << 8;
            red=((pixel & bmp_info.red_mask) << shift.red) >> 16;
            if (quantum_bits.red == 5)
              red|=((red & 0xe000) >> 5);
            if (quantum_bits.red <= 8)
              red|=((red & 0xff00) >> 8);
            green=((pixel & bmp_info.green_mask) << shift.green) >> 16;
            if (quantum_bits.green == 5)
              green|=((green & 0xe000) >> 5);
            if (quantum_bits.green == 6)
              green|=((green & 0xc000) >> 6);
            if (quantum_bits.green <= 8)
              green|=((green & 0xff00) >> 8);
            blue=((pixel & bmp_info.blue_mask) << shift.blue) >> 16;
            if (quantum_bits.blue == 5)
              blue|=((blue & 0xe000) >> 5);
            if (quantum_bits.blue <= 8)
              blue|=((blue & 0xff00) >> 8);
            SetPixelRed(image,ScaleShortToQuantum((unsigned short) red),q);
            SetPixelGreen(image,ScaleShortToQuantum((unsigned short) green),q);
            SetPixelBlue(image,ScaleShortToQuantum((unsigned short) blue),q);
            SetPixelAlpha(image,OpaqueAlpha,q);
            if (image->alpha_trait != UndefinedPixelTrait)
              {
                alpha=((pixel & bmp_info.alpha_mask) << shift.alpha) >> 16;
                if (quantum_bits.alpha <= 8)
                  alpha|=((alpha & 0xff00) >> 8);
                SetPixelAlpha(image,ScaleShortToQuantum(
                  (unsigned short) alpha),q);
              }
            q+=GetPixelChannels(image);
          }
          if (SyncAuthenticPixels(image,exception) == MagickFalse)
            break;
          offset=(MagickOffsetType) (image->rows-y-1);
          if (image->previous == (Image *) NULL)
            {
              status=SetImageProgress(image,LoadImageTag,(MagickOffsetType)
                (image->rows-y),image->rows);
              if (status == MagickFalse)
                break;
            }
        }
        break;
      }
      case 24:
      {
        /*
          Convert DirectColor scanline.
        */
        bytes_per_line=4*((image->columns*24+31)/32);
        for (y=(ssize_t) image->rows-1; y >= 0; y--)
        {
          p=pixels+(image->rows-y-1)*bytes_per_line;
          q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
          if (q == (Quantum *) NULL)
            break;
          for (x=0; x < (ssize_t) image->columns; x++)
          {
            SetPixelBlue(image,ScaleCharToQuantum(*p++),q);
            SetPixelGreen(image,ScaleCharToQuantum(*p++),q);
            SetPixelRed(image,ScaleCharToQuantum(*p++),q);
            SetPixelAlpha(image,OpaqueAlpha,q);
            q+=GetPixelChannels(image);
          }
          if (SyncAuthenticPixels(image,exception) == MagickFalse)
            break;
          offset=(MagickOffsetType) (image->rows-y-1);
          if (image->previous == (Image *) NULL)
            {
              status=SetImageProgress(image,LoadImageTag,(MagickOffsetType)
                (image->rows-y),image->rows);
              if (status == MagickFalse)
                break;
            }
        }
        break;
      }
      case 32:
      {
        /*
          Convert bitfield encoded DirectColor scanline.
        */
        if ((bmp_info.compression != BI_RGB) &&
            (bmp_info.compression != BI_BITFIELDS))
          {
            pixel_info=RelinquishVirtualMemory(pixel_info);
            ThrowReaderException(CorruptImageError,
              "UnrecognizedImageCompression");
          }
        bytes_per_line=4*(image->columns);
        for (y=(ssize_t) image->rows-1; y >= 0; y--)
        {
          unsigned int
            alpha,
            pixel;

          p=pixels+(image->rows-y-1)*bytes_per_line;
          q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
          if (q == (Quantum *) NULL)
            break;
          for (x=0; x < (ssize_t) image->columns; x++)
          {
            pixel=(unsigned int) (*p++);
            pixel|=((unsigned int) *p++ << 8);
            pixel|=((unsigned int) *p++ << 16);
            pixel|=((unsigned int) *p++ << 24);
            red=((pixel & bmp_info.red_mask) << shift.red) >> 16;
            if (quantum_bits.red == 8)
              red|=(red >> 8);
            green=((pixel & bmp_info.green_mask) << shift.green) >> 16;
            if (quantum_bits.green == 8)
              green|=(green >> 8);
            blue=((pixel & bmp_info.blue_mask) << shift.blue) >> 16;
            if (quantum_bits.blue == 8)
              blue|=(blue >> 8);
            SetPixelRed(image,ScaleShortToQuantum((unsigned short) red),q);
            SetPixelGreen(image,ScaleShortToQuantum((unsigned short) green),q);
            SetPixelBlue(image,ScaleShortToQuantum((unsigned short) blue),q);
            SetPixelAlpha(image,OpaqueAlpha,q);
            if (image->alpha_trait != UndefinedPixelTrait)
              {
                alpha=((pixel & bmp_info.alpha_mask) << shift.alpha) >> 16;
                if (quantum_bits.alpha == 8)
                  alpha|=(alpha >> 8);
                SetPixelAlpha(image,ScaleShortToQuantum(
                  (unsigned short) alpha),q);
              }
            q+=GetPixelChannels(image);
          }
          if (SyncAuthenticPixels(image,exception) == MagickFalse)
            break;
          offset=(MagickOffsetType) (image->rows-y-1);
          if (image->previous == (Image *) NULL)
            {
              status=SetImageProgress(image,LoadImageTag,(MagickOffsetType)
                (image->rows-y),image->rows);
              if (status == MagickFalse)
                break;
            }
        }
        break;
      }
      default:
      {
        pixel_info=RelinquishVirtualMemory(pixel_info);
        ThrowReaderException(CorruptImageError,"ImproperImageHeader");
      }
    }
    pixel_info=RelinquishVirtualMemory(pixel_info);
    if (y > 0)
      break;
    if (EOFBlob(image) != MagickFalse)
      {
        ThrowFileException(exception,CorruptImageError,"UnexpectedEndOfFile",
          image->filename);
        break;
      }
    if (bmp_info.height < 0)
      {
        Image
          *flipped_image;

        /*
          Correct image orientation.
        */
        flipped_image=FlipImage(image,exception);
        if (flipped_image != (Image *) NULL)
          {
            DuplicateBlob(flipped_image,image);
            ReplaceImageInList(&image, flipped_image);
            image=flipped_image;
          }
      }
    /*
      Proceed to next image.
    */
    if (image_info->number_scenes != 0)
      if (image->scene >= (image_info->scene+image_info->number_scenes-1))
        break;
    *magick='\0';
    if (bmp_info.ba_offset != 0)
      {
        offset=SeekBlob(image,(MagickOffsetType) bmp_info.ba_offset,SEEK_SET);
        if (offset < 0)
          ThrowReaderException(CorruptImageError,"ImproperImageHeader");
      }
    count=ReadBlob(image,2,magick);
    if ((count == 2) && (IsBMP(magick,2) != MagickFalse))
      {
        /*
          Acquire next image structure.
        */
        AcquireNextImage(image_info,image,exception);
        if (GetNextImageInList(image) == (Image *) NULL)
          {
            status=MagickFalse;
            return((Image *) NULL);
          }
        image=SyncNextImageInList(image);
        status=SetImageProgress(image,LoadImagesTag,TellBlob(image),
          GetBlobSize(image));
        if (status == MagickFalse)
          break;
      }
  } while (IsBMP(magick,2) != MagickFalse);
  (void) CloseBlob(image);
  if (status == MagickFalse)
    return(DestroyImageList(image));
  return(GetFirstImageInList(image));
}
