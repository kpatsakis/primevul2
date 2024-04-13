static MOBI_RET mobi_parse_idxt(MOBIBuffer *buf, MOBIIdxt *idxt, const size_t entries_count) {
    const uint32_t idxt_offset = (uint32_t) buf->offset;
    idxt->offsets_count = 0;
    char idxt_magic[5];
    mobi_buffer_getstring(idxt_magic, buf, 4);
    if (strncmp(idxt_magic, IDXT_MAGIC, 4) != 0) {
        debug_print("IDXT wrong magic: %s\n", idxt_magic);
        return MOBI_DATA_CORRUPT;
    }
    size_t i = 0;
    while (i < entries_count) {
        /* entry offsets */
        idxt->offsets[i++] = mobi_buffer_get16(buf);
    }
    /* last entry end position is IDXT tag offset */
    idxt->offsets[i] = idxt_offset;
    idxt->offsets_count = i;
    return MOBI_SUCCESS;
}