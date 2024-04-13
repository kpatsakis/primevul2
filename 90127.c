u32 h264bsdStorePicParamSet(storage_t *pStorage, picParamSet_t *pPicParamSet)
{

/* Variables */

    u32 id;

/* Code */

    ASSERT(pStorage);
    ASSERT(pPicParamSet);
    ASSERT(pPicParamSet->picParameterSetId < MAX_NUM_PIC_PARAM_SETS);
    ASSERT(pPicParamSet->seqParameterSetId < MAX_NUM_SEQ_PARAM_SETS);

    id = pPicParamSet->picParameterSetId;

 /* pic parameter set with id not used before -> allocate memory */
 if (pStorage->pps[id] == NULL)
 {
        ALLOCATE(pStorage->pps[id], 1, picParamSet_t);
 if (pStorage->pps[id] == NULL)
 return(MEMORY_ALLOCATION_ERROR);
 }
 /* picture parameter set with id equal to id of active pps */
 else if (id == pStorage->activePpsId)
 {
 /* check whether seq param set changes, force re-activation of
         * param set if it does. Set activeSpsId to invalid value to
         * accomplish this */
 if (pPicParamSet->seqParameterSetId != pStorage->activeSpsId)
 {
            pStorage->activePpsId = MAX_NUM_PIC_PARAM_SETS + 1;
 }
 /* free memories allocated for old param set */
        FREE(pStorage->pps[id]->runLength);
        FREE(pStorage->pps[id]->topLeft);
        FREE(pStorage->pps[id]->bottomRight);
        FREE(pStorage->pps[id]->sliceGroupId);
 }
 /* overwrite pic param set other than active one -> free memories
     * allocated for old param set */
 else
 {
        FREE(pStorage->pps[id]->runLength);
        FREE(pStorage->pps[id]->topLeft);
        FREE(pStorage->pps[id]->bottomRight);
        FREE(pStorage->pps[id]->sliceGroupId);
 }

 *pStorage->pps[id] = *pPicParamSet;

 return(HANTRO_OK);

}
