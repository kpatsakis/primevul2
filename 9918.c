static MOBI_RET mobi_parse_ordt(MOBIBuffer *buf, MOBIOrdt *ordt) {
    /* read ORDT1 */
    mobi_buffer_setpos(buf, ordt->ordt1_pos);
    if (mobi_buffer_match_magic(buf, ORDT_MAGIC)) {
        debug_print("%s\n", "ORDT1 section found");
        mobi_buffer_seek(buf, 4);
        if (ordt->offsets_count + buf->offset > buf->maxlen) {
            debug_print("ORDT1 section too long (%zu)", ordt->offsets_count);
            return MOBI_DATA_CORRUPT;
        }
        ordt->ordt1 = malloc(ordt->offsets_count * sizeof(*ordt->ordt1));
        if (ordt->ordt1 == NULL) {
            debug_print("%s", "Memory allocation failed for ORDT1 offsets\n");
            return MOBI_MALLOC_FAILED;
        }
        size_t i = 0;
        while (i < ordt->offsets_count) {
            ordt->ordt1[i++] = mobi_buffer_get8(buf);
        }
        debug_print("ORDT1: read %zu entries\n", ordt->offsets_count);
    }
    /* read ORDT2 */
    mobi_buffer_setpos(buf, ordt->ordt2_pos);
    if (mobi_buffer_match_magic(buf, ORDT_MAGIC)) {
        debug_print("%s\n", "ORDT2 section found");
        mobi_buffer_seek(buf, 4);
        if (ordt->offsets_count * 2 + buf->offset > buf->maxlen) {
            debug_print("ORDT2 section too long (%zu)", ordt->offsets_count);
            return MOBI_DATA_CORRUPT;
        }
        ordt->ordt2 = malloc(ordt->offsets_count * sizeof(*ordt->ordt2));
        if (ordt->ordt2 == NULL) {
            debug_print("%s", "Memory allocation failed for ORDT2 offsets\n");
            return MOBI_MALLOC_FAILED;
        }
        size_t i = 0;
        while (i < ordt->offsets_count) {
            ordt->ordt2[i++] = mobi_buffer_get16(buf);
        }
        debug_print("ORDT2: read %zu entries\n", ordt->offsets_count);
    }
    return MOBI_SUCCESS;
}