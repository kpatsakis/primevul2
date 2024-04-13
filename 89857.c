static EAS_RESULT Parse_wsmp (SDLS_SYNTHESIZER_DATA *pDLSData, EAS_I32 pos, S_WSMP_DATA *p)
{
    EAS_RESULT result;
    EAS_U16 wtemp;
    EAS_U32 ltemp;
    EAS_U32 cbSize;

 /* seek to start of chunk */
 if ((result = EAS_HWFileSeek(pDLSData->hwInstData, pDLSData->fileHandle, pos)) != EAS_SUCCESS)
 return result;

 /* get structure size */
 if ((result = EAS_HWGetDWord(pDLSData->hwInstData, pDLSData->fileHandle, &cbSize, EAS_FALSE)) != EAS_SUCCESS)
 return result;

 /* get unity note */
 if ((result = EAS_HWGetWord(pDLSData->hwInstData, pDLSData->fileHandle, &wtemp, EAS_FALSE)) != EAS_SUCCESS)
 return result;
 if (wtemp <= 127)
        p->unityNote = (EAS_U8) wtemp;
 else
 {
        p->unityNote = 60;
 { /* dpp: EAS_ReportEx(_EAS_SEVERITY_WARNING, "Invalid unity note [%u] in DLS wsmp ignored, set to 60\n", wtemp); */ }
 }

 /* get fine tune */
 if ((result = EAS_HWGetWord(pDLSData->hwInstData, pDLSData->fileHandle, &p->fineTune, EAS_FALSE)) != EAS_SUCCESS)
 return result;

 /* get gain */
 if ((result = EAS_HWGetDWord(pDLSData->hwInstData, pDLSData->fileHandle, &p->gain, EAS_FALSE)) != EAS_SUCCESS)
 return result;
 if (p->gain > 0)
 {
 { /* dpp: EAS_ReportEx(_EAS_SEVERITY_WARNING, "Positive gain [%ld] in DLS wsmp ignored, set to 0dB\n", p->gain); */ }
        p->gain = 0;
 }

 /* option flags */
 if ((result = EAS_HWGetDWord(pDLSData->hwInstData, pDLSData->fileHandle, &ltemp, EAS_FALSE)) != EAS_SUCCESS)
 return result;

 /* sample loops */
 if ((result = EAS_HWGetDWord(pDLSData->hwInstData, pDLSData->fileHandle, &ltemp, EAS_FALSE)) != EAS_SUCCESS)
 return result;

 /* if looped sample, get loop data */
 if (ltemp)
 {

 if (ltemp > 1)
 { /* dpp: EAS_ReportEx(_EAS_SEVERITY_WARNING, "DLS sample with %lu loops, ignoring extra loops\n", ltemp); */ }

 /* skip ahead to loop data */
 if ((result = EAS_HWFileSeek(pDLSData->hwInstData, pDLSData->fileHandle, pos + (EAS_I32) cbSize)) != EAS_SUCCESS)
 return result;

 /* get structure size */
 if ((result = EAS_HWGetDWord(pDLSData->hwInstData, pDLSData->fileHandle, &ltemp, EAS_FALSE)) != EAS_SUCCESS)
 return result;

 /* get loop type */
 if ((result = EAS_HWGetDWord(pDLSData->hwInstData, pDLSData->fileHandle, &ltemp, EAS_FALSE)) != EAS_SUCCESS)
 return result;

 /* get loop start */
 if ((result = EAS_HWGetDWord(pDLSData->hwInstData, pDLSData->fileHandle, &p->loopStart, EAS_FALSE)) != EAS_SUCCESS)
 return result;

 /* get loop length */
 if ((result = EAS_HWGetDWord(pDLSData->hwInstData, pDLSData->fileHandle, &p->loopLength, EAS_FALSE)) != EAS_SUCCESS)
 return result;

 /* ensure no overflow */
 if (p->loopLength
 && ((p->loopStart > EAS_U32_MAX - p->loopLength)
 || (p->loopStart + p->loopLength > EAS_U32_MAX / sizeof(EAS_SAMPLE))))
 {
 return EAS_FAILURE;
 }
 }

 return EAS_SUCCESS;
}
