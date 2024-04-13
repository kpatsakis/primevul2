char * mobi_get_cncx_string_flat(const MOBIPdbRecord *cncx_record, const uint32_t cncx_offset, const size_t length) {
    /* TODO: handle multiple cncx records */
    MOBIBuffer *buf = mobi_buffer_init_null(cncx_record->data, cncx_record->size);
    if (buf == NULL) {
        debug_print("%s\n", "Memory allocation failed");
        return NULL;
    }
    mobi_buffer_setpos(buf, cncx_offset);
    char *string = malloc(length + 1);
    if (string) {
        mobi_buffer_getstring(string, buf, length);
    }
    mobi_buffer_free_null(buf);
    return string;
}