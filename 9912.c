static MOBI_RET mobi_parse_tagx(MOBIBuffer *buf, MOBITagx *tagx) {
    tagx->control_byte_count = 0;
    tagx->tags_count = 0;
    tagx->tags = NULL;
    mobi_buffer_seek(buf, 4); /* skip header */
    uint32_t tagx_record_length = mobi_buffer_get32(buf);
    if (tagx_record_length < 12) {
        debug_print("INDX record too short: %u\n", tagx_record_length);
        return MOBI_DATA_CORRUPT;
    }
    tagx->control_byte_count = mobi_buffer_get32(buf);
    tagx_record_length -= 12;
    if (tagx_record_length + buf->offset > buf->maxlen) {
        debug_print("INDX record too long: %u\n", tagx_record_length);
        return MOBI_DATA_CORRUPT;
    }
    tagx->tags = malloc(tagx_record_length * sizeof(TAGXTags));
    if (tagx->tags == NULL) {
        debug_print("%s", "Memory allocation failed for TAGX tags\n");
        return MOBI_MALLOC_FAILED;
    }
    size_t i = 0;
    const size_t tagx_data_length = tagx_record_length / 4;
    size_t control_byte_count = 0;
    while (i < tagx_data_length) {
        tagx->tags[i].tag = mobi_buffer_get8(buf);
        tagx->tags[i].values_count = mobi_buffer_get8(buf);
        tagx->tags[i].bitmask = mobi_buffer_get8(buf);
        const uint8_t control_byte = mobi_buffer_get8(buf);
        if (control_byte) { control_byte_count++; }
        tagx->tags[i].control_byte = control_byte;
        debug_print("tagx[%zu]:\t%i\t%i\t%i\t%i\n", i, tagx->tags[i].tag, tagx->tags[i].values_count, tagx->tags[i].bitmask, control_byte);
        i++;
    }
    if (tagx->control_byte_count != control_byte_count) {
        debug_print("Wrong count of control bytes: %zu != %zu\n", tagx->control_byte_count, control_byte_count);
        free(tagx->tags);
        tagx->tags = NULL;
        tagx->control_byte_count = 0;
        return MOBI_DATA_CORRUPT;
    }
    tagx->tags_count = i;
    return MOBI_SUCCESS;
}