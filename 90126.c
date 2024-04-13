void h264bsdResetStorage(storage_t *pStorage)
{

/* Variables */

    u32 i;

/* Code */

    ASSERT(pStorage);

    pStorage->slice->numDecodedMbs = 0;
    pStorage->slice->sliceId = 0;

 for (i = 0; i < pStorage->picSizeInMbs; i++)
 {
        pStorage->mb[i].sliceId = 0;
        pStorage->mb[i].decoded = 0;
 }

}
