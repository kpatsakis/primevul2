static MagickStatusType ReadPSDLayers(Image *image,const ImageInfo *image_info,
  const PSDInfo *psd_info,const MagickBooleanType skip_layers,
  ExceptionInfo *exception)
{
  char
    type[4];

  LayerInfo
    *layer_info;

  MagickSizeType
    size;

  MagickStatusType
    status;

  register ssize_t
    i;

  ssize_t
    count,
    j,
    number_layers;

  size=GetPSDSize(psd_info,image);
  if (size == 0)
    {
      size_t
        quantum;

      /*
        Skip layers & masks.
      */
      quantum=psd_info->version == 1 ? 4UL : 8UL;
      (void) ReadBlobMSBLong(image);
      count=ReadBlob(image,4,(unsigned char *) type);
      if ((count == 0) || (LocaleNCompare(type,"8BIM",4) != 0))
        {
          if (DiscardBlobBytes(image,(MagickSizeType) (size-quantum-8)) == MagickFalse)
            ThrowFileException(exception,CorruptImageError,
              "UnexpectedEndOfFile",image->filename);
        }
      else
        {
          count=ReadBlob(image,4,(unsigned char *) type);
          if ((count != 0) && (LocaleNCompare(type,"Lr16",4) == 0))
            size=GetPSDSize(psd_info,image);
          else
            if (DiscardBlobBytes(image,(MagickSizeType) (size-quantum-12)) == MagickFalse)
              ThrowFileException(exception,CorruptImageError,
                "UnexpectedEndOfFile",image->filename);
        }
    }

  status=MagickTrue;
  if (size != 0)
    {
      layer_info=(LayerInfo *) NULL;
      number_layers=(short) ReadBlobMSBShort(image);

      if (number_layers < 0)
        {
          /*
            The first alpha channel in the merged result contains the
            transparency data for the merged result.
          */
          number_layers=MagickAbsoluteValue(number_layers);
          if (image->debug != MagickFalse)
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "  negative layer count corrected for");
          image->matte=MagickTrue;
        }

      if (skip_layers != MagickFalse)
        return(MagickTrue);

      if (image->debug != MagickFalse)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "  image contains %.20g layers",(double) number_layers);

      if (number_layers == 0)
        return(MagickFalse);

      layer_info=(LayerInfo *) AcquireQuantumMemory((size_t) number_layers,
        sizeof(*layer_info));
      if (layer_info == (LayerInfo *) NULL)
        {
          if (image->debug != MagickFalse)
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "  allocation of LayerInfo failed");
          ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
            image->filename);
        }
      (void) ResetMagickMemory(layer_info,0,(size_t) number_layers*
        sizeof(*layer_info));

      for (i=0; i < number_layers; i++)
      {
        int
          x,
          y;

        if (image->debug != MagickFalse)
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "  reading layer #%.20g",(double) i+1);
        layer_info[i].page.y=(int) ReadBlobMSBLong(image);
        layer_info[i].page.x=(int) ReadBlobMSBLong(image);
        y=(int) ReadBlobMSBLong(image);
        x=(int) ReadBlobMSBLong(image);
        layer_info[i].page.width=(ssize_t) (x-layer_info[i].page.x);
        layer_info[i].page.height=(ssize_t) (y-layer_info[i].page.y);
        layer_info[i].channels=ReadBlobMSBShort(image);
        if (layer_info[i].channels > MaxPSDChannels)
          {
            layer_info=DestroyLayerInfo(layer_info,number_layers);
            ThrowBinaryException(CorruptImageError,"MaximumChannelsExceeded",
            image->filename);
          }
        if (image->debug != MagickFalse)
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "    offset(%.20g,%.20g), size(%.20g,%.20g), channels=%.20g",
            (double) layer_info[i].page.x,(double) layer_info[i].page.y,
            (double) layer_info[i].page.height,(double)
            layer_info[i].page.width,(double) layer_info[i].channels);
        for (j=0; j < (ssize_t) layer_info[i].channels; j++)
        {
          layer_info[i].channel_info[j].type=(short) ReadBlobMSBShort(image);
          layer_info[i].channel_info[j].size=(size_t) GetPSDSize(psd_info,
            image);
          if (image->debug != MagickFalse)
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "    channel[%.20g]: type=%.20g, size=%.20g",(double) j,
              (double) layer_info[i].channel_info[j].type,
              (double) layer_info[i].channel_info[j].size);
        }
        count=ReadBlob(image,4,(unsigned char *) type);
        if ((count == 0) || (LocaleNCompare(type,"8BIM",4) != 0))
          {
            if (image->debug != MagickFalse)
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                "  layer type was %.4s instead of 8BIM", type);
            layer_info=DestroyLayerInfo(layer_info,number_layers);
            ThrowBinaryException(CorruptImageError,"ImproperImageHeader",
              image->filename);
          }
        count=ReadBlob(image,4,(unsigned char *) layer_info[i].blendkey);
        layer_info[i].opacity=(Quantum) (QuantumRange-ScaleCharToQuantum(
          (unsigned char) ReadBlobByte(image)));
        layer_info[i].clipping=(unsigned char) ReadBlobByte(image);
        layer_info[i].flags=(unsigned char) ReadBlobByte(image);
        layer_info[i].visible=!(layer_info[i].flags & 0x02);
        if (image->debug != MagickFalse)
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "   blend=%.4s, opacity=%.20g, clipping=%s, flags=%d, visible=%s",
            layer_info[i].blendkey,(double) layer_info[i].opacity,
            layer_info[i].clipping ? "true" : "false",layer_info[i].flags,
            layer_info[i].visible ? "true" : "false");
        (void) ReadBlobByte(image);  /* filler */

        size=ReadBlobMSBLong(image);
        if (size != 0)
          {
            MagickSizeType
              combined_length,
              length;

            if (image->debug != MagickFalse)
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                "    layer contains additional info");
            length=ReadBlobMSBLong(image);
            combined_length=length+4;
            if (length != 0)
              {
                /*
                  Layer mask info.
                */
                layer_info[i].mask.y=(int) ReadBlobMSBLong(image);
                layer_info[i].mask.x=(int) ReadBlobMSBLong(image);
                layer_info[i].mask.height=(size_t)
                  (ReadBlobMSBLong(image)-layer_info[i].mask.y);
                layer_info[i].mask.width=(size_t)
                  (ReadBlobMSBLong(image)-layer_info[i].mask.x);
                if (image->debug != MagickFalse)
                  (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                    "      layer mask: offset(%.20g,%.20g), size(%.20g,%.20g), length=%.20g",
                    (double) layer_info[i].mask.x,(double) 
                    layer_info[i].mask.y,(double) layer_info[i].mask.width,
                    (double) layer_info[i].mask.height,(double)
                    ((MagickOffsetType) length)-16);
                /*
                  Skip over the rest of the layer mask information.
                */
                if (DiscardBlobBytes(image,(MagickSizeType) (length-16)) == MagickFalse)
                  {
                    layer_info=DestroyLayerInfo(layer_info,number_layers);
                    ThrowFileException(exception,CorruptImageError,
                      "UnexpectedEndOfFile",image->filename);
                  }
              }
            length=ReadBlobMSBLong(image);
            combined_length+=length+4;
            if (length != 0)
              {
                /*
                  Layer blending ranges info.
                */
                if (image->debug != MagickFalse)
                  (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                    "      layer blending ranges: length=%.20g",(double)
                    ((MagickOffsetType) length));
                /*
                  We read it, but don't use it...
                */
                for (j=0; j < (ssize_t) (length); j+=8)
                {
                  size_t blend_source=ReadBlobMSBLong(image);
                  size_t blend_dest=ReadBlobMSBLong(image);
                  if (image->debug != MagickFalse)
                    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                      "        source(%x), dest(%x)",(unsigned int)
                      blend_source,(unsigned int) blend_dest);
                }
              }
            /*
              Layer name.
            */
            length=(size_t) ReadBlobByte(image);
            combined_length+=length+1;
            for (j=0; j < (ssize_t) length; j++)
              layer_info[i].name[j]=(unsigned char) ReadBlobByte(image);
            layer_info[i].name[j]='\0';
            if (image->debug != MagickFalse)
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                "      layer name: %s",layer_info[i].name);
            /*
               Skip the rest of the variable data until we support it.
             */
             if (image->debug != MagickFalse)
               (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                 "      unsupported data: length=%.20g",(double)
                 ((MagickOffsetType) (size-combined_length)));
             if (DiscardBlobBytes(image,(MagickSizeType) (size-combined_length)) == MagickFalse)
               {
                 layer_info=DestroyLayerInfo(layer_info,number_layers);
                 ThrowBinaryException(CorruptImageError,
                   "UnexpectedEndOfFile",image->filename);
               }
          }
      }

      for (i=0; i < number_layers; i++)
      {
        if ((layer_info[i].page.width == 0) ||
              (layer_info[i].page.height == 0))
          {
            if (image->debug != MagickFalse)
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                "      layer data is empty");
            continue;
          }

        /*
          Allocate layered image.
        */
        layer_info[i].image=CloneImage(image,layer_info[i].page.width,
          layer_info[i].page.height,MagickFalse,exception);
        if (layer_info[i].image == (Image *) NULL)
          {
            layer_info=DestroyLayerInfo(layer_info,number_layers);
            if (image->debug != MagickFalse)
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                "  allocation of image for layer %.20g failed",(double) i);
            ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
              image->filename);
          }
      }

      if (image_info->ping == MagickFalse)
        {
          for (i=0; i < number_layers; i++)
          {
            if (layer_info[i].image == (Image *) NULL)
              {
                for (j=0; j < layer_info[i].channels; j++)
                {
                  if (DiscardBlobBytes(image,(MagickSizeType)
                      layer_info[i].channel_info[j].size) == MagickFalse)
                    {
                      layer_info=DestroyLayerInfo(layer_info,number_layers);
                      ThrowBinaryException(CorruptImageError,
                        "UnexpectedEndOfFile",image->filename);
                    }
                }
                continue;
              }

            if (image->debug != MagickFalse)
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                "  reading data for layer %.20g",(double) i);
                status=ReadPSDLayer(image,psd_info,&layer_info[i],exception);
                if (status == MagickFalse)
                  break;

            status=SetImageProgress(image,LoadImagesTag,i,(MagickSizeType)
              number_layers);
            if (status == MagickFalse)
              break;
          }
        }

      if (status != MagickFalse)
      {
        for (i=0; i < number_layers; i++)
        {
          if (layer_info[i].image == (Image *) NULL)
          {
            for (j=i; j < number_layers - 1; j++)
              layer_info[j] = layer_info[j+1];
            number_layers--;
            i--;
          }
        }

        if (number_layers > 0)
          {
            for (i=0; i < number_layers; i++)
            {
              if (i > 0)
                layer_info[i].image->previous=layer_info[i-1].image;
              if (i < (number_layers-1))
                layer_info[i].image->next=layer_info[i+1].image;
              layer_info[i].image->page=layer_info[i].page;
            }
            image->next=layer_info[0].image;
            layer_info[0].image->previous=image;
          }
      }
      layer_info=(LayerInfo *) RelinquishMagickMemory(layer_info);
    }

  return(status);
}
