  tt_cmap12_char_index( TT_CMap    cmap,
                        FT_UInt32  char_code )
  {
    return tt_cmap12_char_map_binary( cmap, &char_code, 0 );
  }
