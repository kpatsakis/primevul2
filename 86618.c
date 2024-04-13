void IDNSpoofChecker::SetAllowedUnicodeSet(UErrorCode* status) {
  if (U_FAILURE(*status))
    return;

  const icu::UnicodeSet* recommended_set =
      uspoof_getRecommendedUnicodeSet(status);
  icu::UnicodeSet allowed_set;
  allowed_set.addAll(*recommended_set);
  const icu::UnicodeSet* inclusion_set = uspoof_getInclusionUnicodeSet(status);
  allowed_set.addAll(*inclusion_set);

  allowed_set.remove(0x338u);

  allowed_set.remove(0x58au);  // Armenian Hyphen
  allowed_set.remove(0x2010u);
  allowed_set.remove(0x2019u);  // Right Single Quotation Mark
  allowed_set.remove(0x2027u);
  allowed_set.remove(0x30a0u);  // Katakana-Hiragana Double Hyphen

  allowed_set.remove(0x2bbu);  // Modifier Letter Turned Comma
  allowed_set.remove(0x2bcu);  // Modifier Letter Apostrophe

#if defined(OS_MACOSX)

  allowed_set.remove(0x0620u);

  allowed_set.remove(0x0F8Cu);
  allowed_set.remove(0x0F8Du);
  allowed_set.remove(0x0F8Eu);
  allowed_set.remove(0x0F8Fu);
#endif

  uspoof_setAllowedUnicodeSet(checker_, &allowed_set, status);
}
