u32 h264bsdIsEndOfPicture(storage_t *pStorage)
{

/* Variables */

    u32 i, tmp;

/* Code */

 /* primary picture */
 if (!pStorage->sliceHeader[0].redundantPicCnt)
 {
 if (pStorage->slice->numDecodedMbs == pStorage->picSizeInMbs)
 return(HANTRO_TRUE);
 }
 else
 {
 for (i = 0, tmp = 0; i < pStorage->picSizeInMbs; i++)
            tmp += pStorage->mb[i].decoded ? 1 : 0;

 if (tmp == pStorage->picSizeInMbs)
 return(HANTRO_TRUE);
 }

 return(HANTRO_FALSE);

}
