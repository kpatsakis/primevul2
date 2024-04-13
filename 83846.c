png_set_crc_action(png_structp png_ptr, int crit_action, int ancil_action)
{
   png_debug(1, "in png_set_crc_action");
 
   if (png_ptr == NULL)
      return;

   /* Tell libpng how we react to CRC errors in critical chunks */
   switch (crit_action)
   {
      case PNG_CRC_NO_CHANGE:                        /* Leave setting as is */
         break;

      case PNG_CRC_WARN_USE:                               /* Warn/use data */
         png_ptr->flags &= ~PNG_FLAG_CRC_CRITICAL_MASK;
         png_ptr->flags |= PNG_FLAG_CRC_CRITICAL_USE;
         break;

      case PNG_CRC_QUIET_USE:                             /* Quiet/use data */
         png_ptr->flags &= ~PNG_FLAG_CRC_CRITICAL_MASK;
         png_ptr->flags |= PNG_FLAG_CRC_CRITICAL_USE |
                           PNG_FLAG_CRC_CRITICAL_IGNORE;
         break;

      case PNG_CRC_WARN_DISCARD:    /* Not a valid action for critical data */
         png_warning(png_ptr,
            "Can't discard critical data on CRC error.");
      case PNG_CRC_ERROR_QUIT:                                /* Error/quit */

      case PNG_CRC_DEFAULT:
      default:
         png_ptr->flags &= ~PNG_FLAG_CRC_CRITICAL_MASK;
         break;
   }

   /* Tell libpng how we react to CRC errors in ancillary chunks */
   switch (ancil_action)
   {
      case PNG_CRC_NO_CHANGE:                       /* Leave setting as is */
         break;

      case PNG_CRC_WARN_USE:                              /* Warn/use data */
         png_ptr->flags &= ~PNG_FLAG_CRC_ANCILLARY_MASK;
         png_ptr->flags |= PNG_FLAG_CRC_ANCILLARY_USE;
         break;

      case PNG_CRC_QUIET_USE:                            /* Quiet/use data */
         png_ptr->flags &= ~PNG_FLAG_CRC_ANCILLARY_MASK;
         png_ptr->flags |= PNG_FLAG_CRC_ANCILLARY_USE |
                           PNG_FLAG_CRC_ANCILLARY_NOWARN;
         break;

      case PNG_CRC_ERROR_QUIT:                               /* Error/quit */
         png_ptr->flags &= ~PNG_FLAG_CRC_ANCILLARY_MASK;
         png_ptr->flags |= PNG_FLAG_CRC_ANCILLARY_NOWARN;
         break;

      case PNG_CRC_WARN_DISCARD:                      /* Warn/discard data */

      case PNG_CRC_DEFAULT:
      default:
         png_ptr->flags &= ~PNG_FLAG_CRC_ANCILLARY_MASK;
         break;
   }
}
