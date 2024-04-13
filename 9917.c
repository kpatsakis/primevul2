uint32_t mobi_get_orth_entry_offset(const MOBIIndexEntry *entry) {

    uint32_t entry_startpos;
    MOBI_RET ret = mobi_get_indxentry_tagvalue(&entry_startpos, entry, INDX_TAG_ORTH_POSITION);
    if (ret != MOBI_SUCCESS) {
        return MOBI_NOTSET;
    }
    
    return entry_startpos;
}