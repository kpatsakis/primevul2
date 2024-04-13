MOBI_RET mobi_parse_index(const MOBIData *m, MOBIIndx *indx, const size_t indx_record_number) {
    MOBI_RET ret;
    /* tagx->tags array will be allocated in mobi_parse_tagx */
    MOBITagx *tagx = calloc(1, sizeof(MOBITagx));
    if (tagx == NULL) {
        mobi_free_indx(indx);
        debug_print("%s\n", "Memory allocation failed");
        return MOBI_MALLOC_FAILED;
    }
    /* ordt->ordt1 and ordt.ordt2 arrays will be allocated in mobi_parse_ordt */
    MOBIOrdt *ordt = calloc(1, sizeof(MOBIOrdt));
    if (ordt == NULL) {
        mobi_free_indx(indx);
        mobi_free_tagx(tagx);
        debug_print("%s\n", "Memory allocation failed");
        return MOBI_MALLOC_FAILED;
    }
    /* parse first meta INDX record */
    MOBIPdbRecord *record = mobi_get_record_by_seqnumber(m, indx_record_number);
    ret = mobi_parse_indx(record, indx, tagx, ordt);
    if (ret != MOBI_SUCCESS) {
        mobi_free_indx(indx);
        mobi_free_tagx(tagx);
        mobi_free_ordt(ordt);
        return ret;
    }
    /* parse remaining INDX records for the index */
    size_t count = indx->entries_count;
    indx->entries_count = 0;
    while (count--) {
        record = record->next;
        ret = mobi_parse_indx(record, indx, tagx, ordt);
        if (ret != MOBI_SUCCESS) {
            mobi_free_indx(indx);
            mobi_free_tagx(tagx);
            mobi_free_ordt(ordt);
            return ret;
        }
    }
    if (indx->entries_count != indx->total_entries_count) {
        debug_print("Entries count %zu != total entries count %zu\n", indx->entries_count, indx->total_entries_count);
        mobi_free_indx(indx);
        mobi_free_tagx(tagx);
        mobi_free_ordt(ordt);
        return MOBI_DATA_CORRUPT;
    }
    /* copy pointer to first cncx record if present and set info from first record */
    if (indx->cncx_records_count) {
        indx->cncx_record = record->next;
    }
    mobi_free_tagx(tagx);
    mobi_free_ordt(ordt);
    return MOBI_SUCCESS;
}