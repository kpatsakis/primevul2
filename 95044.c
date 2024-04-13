  tt_cmap13_char_map_binary( TT_CMap     cmap,
                             FT_UInt32*  pchar_code,
                             FT_Bool     next )
  {
    FT_UInt    gindex     = 0;
    FT_Byte*   p          = cmap->data + 12;
    FT_UInt32  num_groups = TT_PEEK_ULONG( p );
    FT_UInt32  char_code  = *pchar_code;
    FT_UInt32  start, end;
    FT_UInt32  max, min, mid;


    if ( !num_groups )
      return 0;

    /* make compiler happy */
    mid = num_groups;
    end = 0xFFFFFFFFUL;

    if ( next )
      char_code++;

    min = 0;
    max = num_groups;

    /* binary search */
    while ( min < max )
    {
      mid = ( min + max ) >> 1;
      p   = cmap->data + 16 + 12 * mid;

      start = TT_NEXT_ULONG( p );
      end   = TT_NEXT_ULONG( p );

      if ( char_code < start )
        max = mid;
      else if ( char_code > end )
        min = mid + 1;
      else
      {
        gindex = (FT_UInt)TT_PEEK_ULONG( p );

        break;
      }
    }

    if ( next )
    {
      TT_CMap13  cmap13 = (TT_CMap13)cmap;


      /* if `char_code' is not in any group, then `mid' is */
      /* the group nearest to `char_code'                  */

      if ( char_code > end )
      {
        mid++;
        if ( mid == num_groups )
          return 0;
      }

      cmap13->valid        = 1;
      cmap13->cur_charcode = char_code;
      cmap13->cur_group    = mid;

      if ( !gindex )
      {
        tt_cmap13_next( cmap13 );

        if ( cmap13->valid )
          gindex = cmap13->cur_gindex;
      }
      else
        cmap13->cur_gindex = gindex;

      if ( gindex )
        *pchar_code = cmap13->cur_charcode;
    }

    return gindex;
  }
