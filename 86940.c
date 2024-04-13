base::string16 LocalizeUrl(const wchar_t* url) {
  base::string16 language;
  if (!GoogleUpdateSettings::GetLanguage(&language))
    language = L"en-US";  // Default to US English.
  return base::ReplaceStringPlaceholders(url, language, NULL);
}
