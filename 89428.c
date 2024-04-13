static EAS_RESULT Parse_lrgn (SDLS_SYNTHESIZER_DATA *pDLSData, EAS_I32 pos, EAS_I32 size, EAS_U16 artIndex, EAS_U32 numRegions)
{
    EAS_RESULT result;
    EAS_U32 temp;
    EAS_I32 chunkPos;
    EAS_I32 endChunk;
    EAS_U16 regionCount;

 /* seek to start of chunk */
 if ((result = EAS_HWFileSeek(pDLSData->hwInstData, pDLSData->fileHandle, pos)) != EAS_SUCCESS)
 return result;

 /* read to end of chunk */
    regionCount = 0;
    endChunk = pos + size;
 while (pos < endChunk)
 {
        chunkPos = pos;

 /* get the next chunk type */
 if ((result = NextChunk(pDLSData, &pos, &temp, &size)) != EAS_SUCCESS)
 return result;

 if ((temp == CHUNK_RGN) || (temp == CHUNK_RGN2))
 {
 if (regionCount == numRegions)
 {
 { /* dpp: EAS_ReportEx(_EAS_SEVERITY_WARNING, "DLS region count exceeded cRegions value in insh, extra region ignored\n"); */ }
 return EAS_SUCCESS;
 }
 if ((result = Parse_rgn(pDLSData, chunkPos + 12, size, artIndex)) != EAS_SUCCESS)
 return result;
            regionCount++;
 }
 }

 /* set a flag in the last region */
 if ((pDLSData->pDLS != NULL) && (regionCount > 0))
        pDLSData->pDLS->pDLSRegions[pDLSData->regionCount - 1].wtRegion.region.keyGroupAndFlags |= REGION_FLAG_LAST_REGION;

 return EAS_SUCCESS;
}
