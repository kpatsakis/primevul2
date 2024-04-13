  FT_FSPathMakeRes( const UInt8*    pathname,
                    ResFileRefNum*  res )
  {
    OSErr  err;
    FSRef  ref;


    if ( noErr != FSPathMakeRef( pathname, &ref, FALSE ) )
      return FT_THROW( Cannot_Open_Resource );

    /* at present, no support for dfont format */
    err = FSOpenResourceFile( &ref, 0, NULL, fsRdPerm, res );
    if ( noErr == err )
      return err;

    /* fallback to original resource-fork font */
    *res = FSOpenResFile( &ref, fsRdPerm );
    err  = ResError();

    return err;
  }
