  tt_check_trickyness( FT_Face  face )
  {
    if ( !face )
      return FALSE;

    /* For first, check the face name for quick check. */
    if ( face->family_name                               &&
         tt_check_trickyness_family( face->family_name ) )
      return TRUE;

    /* Type42 fonts may lack `name' tables, we thus try to identify */
    /* tricky fonts by checking the checksums of Type42-persistent  */
    /* sfnt tables (`cvt', `fpgm', and `prep').                     */
    if ( tt_check_trickyness_sfnt_ids( (TT_Face)face ) )
      return TRUE;

    return FALSE;
  }
