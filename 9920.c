size_t mobi_getstring_ordt(const MOBIOrdt *ordt, MOBIBuffer *buf, unsigned char *output, size_t length) {
    size_t i = 0;
    size_t output_length = 0;
    const uint32_t bytemask = 0xbf;
    const uint32_t bytemark = 0x80;
    const uint32_t uni_replacement = 0xfffd;
    const uint32_t surrogate_offset = 0x35fdc00;
    static const uint8_t init_byte[7] = { 0x00, 0x00, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc };
    while (i < length) {
        uint16_t offset;
        i += mobi_ordt_getbuffer(ordt, buf, &offset);
        uint32_t codepoint = mobi_ordt_lookup(ordt, offset);
        if (codepoint <= 5) {
            size_t k = mobi_ordt_getbuffer(ordt, buf, &offset);
            uint32_t codepoint2 = mobi_ordt_lookup(ordt, offset);
            codepoint = mobi_ligature_to_utf16(codepoint, codepoint2);
            if (codepoint == uni_replacement) {
                /* rewind buffer to codepoint2 */
                debug_print("Invalid ligature sequence%s", "\n");
                mobi_buffer_seek(buf, - (int) k);
            } else {
                i += k;
            }
        }
        /* convert UTF-16 surrogates into UTF-32 */
        if (codepoint >= 0xd800 && codepoint <= 0xdbff) {
            size_t k = mobi_ordt_getbuffer(ordt, buf, &offset);
            uint32_t codepoint2 = mobi_ordt_lookup(ordt, offset);
            if (codepoint2 >= 0xdc00 && codepoint2 <= 0xdfff) {
                i += k;
                codepoint = (codepoint << 10) + codepoint2 - surrogate_offset;
            } else {
                /* illegal unpaired high surrogate */
                /* rewind buffer to codepoint2 */
                debug_print("Invalid code point: %u\n", codepoint);
                mobi_buffer_seek(buf, - (int) k);
                codepoint = uni_replacement;
            }
        }
        if ((codepoint >= 0xdc00 && codepoint <= 0xdfff) /* unpaired low surrogate */
            || (codepoint >= 0xfdd0 && codepoint <= 0xfdef) /* invalid characters */
            || (codepoint & 0xfffe) == 0xfffe /* reserved characters */
            || codepoint == 0 /* remove zeroes */) {
            codepoint = uni_replacement;
            debug_print("Invalid code point: %u\n", codepoint);
        }
        /* Conversion routine based on unicode's ConvertUTF.c */
        size_t bytes;
        if (codepoint < 0x80) { bytes = 1; }
        else if (codepoint < 0x800) { bytes = 2; }
        else if (codepoint < 0x10000) { bytes = 3; }
        else if (codepoint < 0x110000) { bytes = 4; }
        else {
            bytes = 3;
            codepoint = uni_replacement;
            debug_print("Invalid code point: %u\n", codepoint);
        }
        if (output_length + bytes >= INDX_LABEL_SIZEMAX) {
            debug_print("%s\n", "INDX label too long");
            break;
        }
        output += bytes;
        switch (bytes) {
            case 4: *--output = (uint8_t)((codepoint | bytemark) & bytemask); codepoint >>= 6; /* falls through */
            case 3: *--output = (uint8_t)((codepoint | bytemark) & bytemask); codepoint >>= 6; /* falls through */
            case 2: *--output = (uint8_t)((codepoint | bytemark) & bytemask); codepoint >>= 6; /* falls through */
            case 1: *--output = (uint8_t)(codepoint | init_byte[bytes]);
        }
        output += bytes;
        output_length += bytes;
    }
    *output = '\0';
    return output_length;
}