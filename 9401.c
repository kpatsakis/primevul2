unicode_fixup_string(char *str, const char *fromcode)
{
  uint8_t *ret;
  size_t len;

  if (!str)
    return NULL;

  len = strlen(str);

  /* String is valid UTF-8 */
  if (!u8_check((uint8_t *)str, len))
    {
      if (len >= 3)
	{
	  /* Check for and strip byte-order mark */
	  if (memcmp("\xef\xbb\xbf", str, 3) == 0)
	    memmove(str, str + 3, len - 3 + 1);
	}

      return str;
    }

  ret = u8_strconv_from_encoding(str, fromcode, iconveh_question_mark);
  if (!ret)
    {
      DPRINTF(E_LOG, L_MISC, "Could not convert string '%s' to UTF-8: %s\n", str, strerror(errno));

      return NULL;
    }

  return (char *)ret;
}