static EAS_RESULT Parse_insh (SDLS_SYNTHESIZER_DATA *pDLSData, EAS_I32 pos, EAS_U32 *pRgnCount, EAS_U32 *pLocale)
{
    EAS_RESULT result;
    EAS_U32 bank;
    EAS_U32 program;

 /* seek to start of chunk */
 if ((result = EAS_HWFileSeek(pDLSData->hwInstData, pDLSData->fileHandle, pos)) != EAS_SUCCESS)
 return result;

 /* get the region count and locale */
 if ((result = EAS_HWGetDWord(pDLSData->hwInstData, pDLSData->fileHandle, pRgnCount, EAS_FALSE)) != EAS_SUCCESS)
 return result;
 if ((result = EAS_HWGetDWord(pDLSData->hwInstData, pDLSData->fileHandle, &bank, EAS_FALSE)) != EAS_SUCCESS)
 return result;
 if ((result = EAS_HWGetDWord(pDLSData->hwInstData, pDLSData->fileHandle, &program, EAS_FALSE)) != EAS_SUCCESS)
 return result;

 /* verify the parameters are valid */
 if (bank & 0x7fff8080)
 {
 { /* dpp: EAS_ReportEx(_EAS_SEVERITY_WARNING, "DLS bank number is out of range: %08lx\n", bank); */ }
        bank &= 0xff7f;
 }
 if (program > 127)
 {
 { /* dpp: EAS_ReportEx(_EAS_SEVERITY_WARNING, "DLS program number is out of range: %08lx\n", program); */ }
        program &= 0x7f;
 }

 /* save the program number */
 *pLocale = (bank << 8) | program;
 return EAS_SUCCESS;
}
