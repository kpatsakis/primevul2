MagickExport MagickBooleanType SetQuantumDepth(const Image *image,
  QuantumInfo *quantum_info,const size_t depth)
{
  size_t
    extent,
    quantum;

  /*
    Allocate the quantum pixel buffer.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(quantum_info != (QuantumInfo *) NULL);
  assert(quantum_info->signature == MagickSignature);
  quantum_info->depth=depth;
  if (quantum_info->format == FloatingPointQuantumFormat)
    {
      if (quantum_info->depth > 32)
        quantum_info->depth=64;
      else
        if (quantum_info->depth > 16)
          quantum_info->depth=32;
        else
          quantum_info->depth=16;
    }
  if (quantum_info->pixels != (unsigned char **) NULL)
     DestroyQuantumPixels(quantum_info);
   quantum=(quantum_info->pad+6)*(quantum_info->depth+7)/8;
   extent=image->columns*quantum;
  if ((image->columns != 0) && (quantum != (extent/image->columns)))
     return(MagickFalse);
   return(AcquireQuantumPixels(quantum_info,extent));
 }
