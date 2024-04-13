char * mobi_get_cncx_string(const MOBIPdbRecord *cncx_record, const uint32_t cncx_offset) {
    /* TODO: handle multiple cncx records */
    MOBIBuffer *buf = mobi_buffer_init_null(cncx_record->data, cncx_record->size);
    if (buf == NULL) {
        debug_print("%s\n", "Memory allocation failed");
        return NULL;
    }
    mobi_buffer_setpos(buf, cncx_offset);
    size_t len = 0;
    const uint32_t string_length = mobi_buffer_get_varlen(buf, &len);
    char *string = malloc(string_length + 1);
    if (string) {
        mobi_buffer_getstring(string, buf, string_length);
    }
    mobi_buffer_free_null(buf);
    return string;
}