static EAS_RESULT Parse_lart (SDLS_SYNTHESIZER_DATA *pDLSData, EAS_I32 pos, EAS_I32 size, S_DLS_ART_VALUES *pArt)
{
    EAS_RESULT result;
    EAS_U32 temp;
    EAS_I32 endChunk;
    EAS_I32 chunkPos;
    EAS_I32 art1Pos;
    EAS_I32 art2Pos;

 /* seek to start of chunk */
 if ((result = EAS_HWFileSeek(pDLSData->hwInstData, pDLSData->fileHandle, pos)) != EAS_SUCCESS)
 return result;

 /* no articulation chunks yet */
    art1Pos = art2Pos = 0;

 /* read to end of chunk */
    endChunk = pos + size;
 while (pos < endChunk)
 {
        chunkPos = pos;

 /* get the next chunk type */
 if ((result = NextChunk(pDLSData, &pos, &temp, &size)) != EAS_SUCCESS)
 return result;

 /* parse useful chunks */
 switch (temp)
 {
 case CHUNK_CDL:
 if ((result = Parse_cdl(pDLSData, size, &temp)) != EAS_SUCCESS)
 return result;

 /* if conditional chunk evaluates false, skip this list */
 if (!temp)
 return EAS_SUCCESS;
 break;

 case CHUNK_ART1:
                art1Pos = chunkPos + 8;
 break;

 case CHUNK_ART2:
                art2Pos = chunkPos + 8;
 break;

 default:
 break;

 }
 }

 if (art1Pos)
 {
 if ((result = Parse_art(pDLSData, art1Pos, pArt)) != EAS_SUCCESS)
 return result;
 }

 if (art2Pos)
 {
 if ((result = Parse_art(pDLSData, art2Pos, pArt)) != EAS_SUCCESS)
 return result;
 }

 return EAS_SUCCESS;
}
