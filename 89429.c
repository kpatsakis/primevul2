static EAS_RESULT Parse_ptbl (SDLS_SYNTHESIZER_DATA *pDLSData, EAS_I32 pos, EAS_I32 wtblPos, EAS_I32 wtblSize)
{
    EAS_RESULT result;
    EAS_U32 temp;
    EAS_FILE_HANDLE tempFile;
    EAS_U16 waveIndex;

 /* seek to start of chunk */
 if ((result = EAS_HWFileSeek(pDLSData->hwInstData, pDLSData->fileHandle, pos)) != EAS_SUCCESS)
 return result;

 /* get the structure size */
 if ((result = EAS_HWGetDWord(pDLSData->hwInstData, pDLSData->fileHandle, &temp, EAS_FALSE)) != EAS_SUCCESS)
 return result;

 /* get the number of waves */
 if ((result = EAS_HWGetDWord(pDLSData->hwInstData, pDLSData->fileHandle, &pDLSData->waveCount, EAS_FALSE)) != EAS_SUCCESS)
 return result;

#if 0
 /* just need the wave count on the first pass */
 if (!pDLSData->pDLS)
 return EAS_SUCCESS;
#endif

 /* open duplicate file handle */
 if ((result = EAS_HWDupHandle(pDLSData->hwInstData, pDLSData->fileHandle, &tempFile)) != EAS_SUCCESS)
 return result;

 /* read to end of chunk */
 for (waveIndex = 0; waveIndex < pDLSData->waveCount; waveIndex++)
 {

 /* get the offset to the wave and make sure it is within the wtbl chunk */
 if ((result = EAS_HWGetDWord(pDLSData->hwInstData, tempFile, &temp, EAS_FALSE)) != EAS_SUCCESS)
 return result;
 if (temp > (EAS_U32) wtblSize)
 {
 { /* dpp: EAS_ReportEx(_EAS_SEVERITY_ERROR, "Ptbl offset exceeds size of wtbl\n"); */ }
            EAS_HWCloseFile(pDLSData->hwInstData, tempFile);
 return EAS_ERROR_FILE_FORMAT;
 }

 /* parse the wave */
 if ((result = Parse_wave(pDLSData, wtblPos +(EAS_I32)  temp, waveIndex)) != EAS_SUCCESS)
 return result;
 }

 /* close the temporary handle and return */
    EAS_HWCloseFile(pDLSData->hwInstData, tempFile);
 return EAS_SUCCESS;
}
