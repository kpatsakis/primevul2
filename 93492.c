 static MagickBooleanType ConcatenateImages(int argc,char **argv,
  ExceptionInfo *exception )
 {
   FILE
     *input,
     *output;
 
  MagickBooleanType
    status;

   int
     c;
 
  register ssize_t
    i;
 
   if (ExpandFilenames(&argc,&argv) == MagickFalse)
     ThrowFileException(exception,ResourceLimitError,"MemoryAllocationFailed",
      GetExceptionMessage(errno));
   output=fopen_utf8(argv[argc-1],"wb");
  if (output == (FILE *) NULL)
    {
      ThrowFileException(exception,FileOpenError,"UnableToOpenFile",
        argv[argc-1]);
      return(MagickFalse);
    }
  status=MagickTrue;
  for (i=2; i < (ssize_t) (argc-1); i++)
  {
     input=fopen_utf8(argv[i],"rb");
    if (input == (FILE *) NULL)
      {
         ThrowFileException(exception,FileOpenError,"UnableToOpenFile",argv[i]);
         continue;
       }
     for (c=fgetc(input); c != EOF; c=fgetc(input))
      if (fputc((char) c,output) != c)
        status=MagickFalse;
     (void) fclose(input);
     (void) remove_utf8(argv[i]);
   }
   (void) fclose(output);
  return(status);
 }
