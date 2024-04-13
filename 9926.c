size_t mobi_get_indxentry_tagarray(uint32_t **tagarr, const MOBIIndexEntry *entry, const size_t tagid) {
    if (entry == NULL) {
        debug_print("%s", "INDX entry not initialized\n");
        return 0;
    }
    size_t i = 0;
    while (i < entry->tags_count) {
        if (entry->tags[i].tagid == tagid) {
            *tagarr = entry->tags[i].tagvalues;
            return entry->tags[i].tagvalues_count;
        }
        i++;
    }
    //debug_print("tag[%zu] not found in entry: %s\n", tagid, entry->label);
    return 0;
}