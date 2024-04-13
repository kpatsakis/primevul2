CMSAPI cmsBool CMSEXPORT cmsMLUgetTranslation(const cmsMLU* mlu,
                                              const char LanguageCode[3], const char CountryCode[3],
                                              char ObtainedLanguage[3], char ObtainedCountry[3])
{
    const wchar_t *Wide;

    cmsUInt16Number Lang  = _cmsAdjustEndianess16(*(cmsUInt16Number*) LanguageCode);
    cmsUInt16Number Cntry = _cmsAdjustEndianess16(*(cmsUInt16Number*) CountryCode);
    cmsUInt16Number ObtLang, ObtCode;

    if (mlu == NULL) return FALSE;

    Wide = _cmsMLUgetWide(mlu, NULL, Lang, Cntry, &ObtLang, &ObtCode);
    if (Wide == NULL) return FALSE;

    *(cmsUInt16Number *)ObtainedLanguage = _cmsAdjustEndianess16(ObtLang);
    *(cmsUInt16Number *)ObtainedCountry  = _cmsAdjustEndianess16(ObtCode);

    ObtainedLanguage[2] = ObtainedCountry[2] = 0;
    return TRUE;
}
