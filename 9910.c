MOBI_RET mobi_get_indxentry_tagvalue(uint32_t *tagvalue, const MOBIIndexEntry *entry, const unsigned tag_arr[]) {
    if (entry == NULL) {
        debug_print("%s", "INDX entry not initialized\n");
        return MOBI_INIT_FAILED;
    }
    size_t i = 0;
    while (i < entry->tags_count) {
        if (entry->tags[i].tagid == tag_arr[0]) {
            if (entry->tags[i].tagvalues_count > tag_arr[1]) {
                *tagvalue = entry->tags[i].tagvalues[tag_arr[1]];
                return MOBI_SUCCESS;
            }
            break;
        }
        i++;
    }
    //debug_print("tag[%i][%i] not found in entry: %s\n", tag_arr[0], tag_arr[1], entry->label);
    return MOBI_DATA_CORRUPT;
}