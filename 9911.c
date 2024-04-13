size_t mobi_indx_get_label(unsigned char *output, MOBIBuffer *buf, const size_t length, const size_t has_ligatures) {
    if (!output) {
        buf->error = MOBI_PARAM_ERR;
        return 0;
    }
    if (buf->offset + length > buf->maxlen) {
        debug_print("%s", "End of buffer\n");
        buf->error = MOBI_BUFFER_END;
        return 0;
    }
    const unsigned char replacement = 0x3f;
    size_t output_length = 0;
    size_t i = 0;
    while (i < length && output_length < INDX_LABEL_SIZEMAX) {
        unsigned char c = mobi_buffer_get8(buf);
        i++;
        if (c == 0) {
            /* FIXME: is it safe to replace zeroes? */
            debug_print("Invalid character: %u\n", c);
            c = replacement;
        }
        if (c <= 5 && has_ligatures) {
            unsigned char c2 = mobi_buffer_get8(buf);
            c = mobi_ligature_to_cp1252(c, c2);
            if (c == 0) {
                debug_print("Invalid ligature sequence%s", "\n");
                mobi_buffer_seek(buf, -1);
                c = replacement;
            } else {
                i++;
            }
        }
        *output++ = c;
        output_length++;
    }
    *output = '\0';
    return output_length;
}