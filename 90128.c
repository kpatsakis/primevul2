u32 h264bsdStoreSeqParamSet(storage_t *pStorage, seqParamSet_t *pSeqParamSet)
{

/* Variables */

    u32 id;

/* Code */

    ASSERT(pStorage);
    ASSERT(pSeqParamSet);
    ASSERT(pSeqParamSet->seqParameterSetId < MAX_NUM_SEQ_PARAM_SETS);

    id = pSeqParamSet->seqParameterSetId;

 /* seq parameter set with id not used before -> allocate memory */
 if (pStorage->sps[id] == NULL)
 {
        ALLOCATE(pStorage->sps[id], 1, seqParamSet_t);
 if (pStorage->sps[id] == NULL)
 return(MEMORY_ALLOCATION_ERROR);
 }
 /* sequence parameter set with id equal to id of active sps */
 else if (id == pStorage->activeSpsId)
 {
 /* if seq parameter set contents changes
         *    -> overwrite and re-activate when next IDR picture decoded
         *    ids of active param sets set to invalid values to force
         *    re-activation. Memories allocated for old sps freed
         * otherwise free memeries allocated for just decoded sps and
         * continue */
 if (h264bsdCompareSeqParamSets(pSeqParamSet, pStorage->activeSps) != 0)
 {
            FREE(pStorage->sps[id]->offsetForRefFrame);
            FREE(pStorage->sps[id]->vuiParameters);
            pStorage->activeSpsId = MAX_NUM_SEQ_PARAM_SETS + 1;
            pStorage->activePpsId = MAX_NUM_PIC_PARAM_SETS + 1;
            pStorage->activeSps = NULL;
            pStorage->activePps = NULL;
 }
 else
 {
            FREE(pSeqParamSet->offsetForRefFrame);
            FREE(pSeqParamSet->vuiParameters);
 return(HANTRO_OK);
 }
 }
 /* overwrite seq param set other than active one -> free memories
     * allocated for old param set */
 else
 {
        FREE(pStorage->sps[id]->offsetForRefFrame);
        FREE(pStorage->sps[id]->vuiParameters);
 }

 *pStorage->sps[id] = *pSeqParamSet;

 return(HANTRO_OK);

}
