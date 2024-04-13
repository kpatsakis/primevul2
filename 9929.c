MOBI_RET mobi_parse_indx(const MOBIPdbRecord *indx_record, MOBIIndx *indx, MOBITagx *tagx, MOBIOrdt *ordt) {
    if (indx_record == NULL || indx == NULL || tagx == NULL || ordt == NULL) {
        debug_print("%s", "index structure not initialized\n");
        return MOBI_INIT_FAILED;
    }
    MOBI_RET ret = MOBI_SUCCESS;
    MOBIBuffer *buf = mobi_buffer_init_null(indx_record->data, indx_record->size);
    if (buf == NULL) {
        debug_print("%s\n", "Memory allocation failed");
        return MOBI_MALLOC_FAILED;
    }
    char indx_magic[5];
    mobi_buffer_getstring(indx_magic, buf, 4); /* 0: INDX magic */
    const uint32_t header_length = mobi_buffer_get32(buf); /* 4: header length */
    if (strncmp(indx_magic, INDX_MAGIC, 4) != 0 ||
        header_length == 0 || header_length > indx_record->size) {
        debug_print("INDX wrong magic: %s or header length: %u\n", indx_magic, header_length);
        mobi_buffer_free_null(buf);
        return MOBI_DATA_CORRUPT;
    }
    mobi_buffer_seek(buf, 4); /* 8: unk, usually zeroes */
    const uint32_t type = mobi_buffer_get32(buf); /* 12: 0 - normal, 2 - inflection */
    mobi_buffer_seek(buf, 4); /* 16: unk */
    const uint32_t idxt_offset = mobi_buffer_get32(buf); /* 20: IDXT offset */
    const uint32_t entries_count = mobi_buffer_get32(buf); /* 24: entries count */
    if (entries_count > INDX_RECORD_MAXCNT) {
        debug_print("Too many index entries (%u)\n", entries_count);
        mobi_buffer_free_null(buf);
        return MOBI_DATA_CORRUPT;
    }
    /* if record contains TAGX section, read it (and ORDT) and return */
    if (mobi_buffer_match_magic_offset(buf, TAGX_MAGIC, header_length) && indx->total_entries_count == 0) {
        buf->maxlen = header_length;
        /* TAGX metadata */
        uint32_t encoding = mobi_buffer_get32(buf); /* 28: encoding */
        if (encoding == MOBI_NOTSET) { encoding = MOBI_CP1252; }
        mobi_buffer_seek(buf, 4); /* 32 */
        const uint32_t total_entries_count = mobi_buffer_get32(buf); /* 36: total entries count */
        if (total_entries_count > INDX_TOTAL_MAXCNT) {
            debug_print("Too many total index entries (%u)\n", total_entries_count);
            mobi_buffer_free_null(buf);
            return MOBI_DATA_CORRUPT;
        }
        uint32_t ordt_offset = mobi_buffer_get32(buf); /* 40: ORDT offset; currently not used */
        if (ordt_offset + ORDT_RECORD_MAXCNT + 4 > indx_record->size) {
            ordt_offset = 0;
        }
        uint32_t ligt_offset = mobi_buffer_get32(buf); /* 44: LIGT offset; currently static table used instead */
        uint32_t ligt_entries_count = mobi_buffer_get32(buf); /* 48: LIGT entries count */
        if (ligt_offset + 4 * ligt_entries_count + 4 > indx_record->size) {
            ligt_offset = 0;
            ligt_entries_count = 0;
        }
        const uint32_t cncx_records_count = mobi_buffer_get32(buf); /* 52: CNCX entries count */
        if (cncx_records_count > CNCX_RECORD_MAXCNT) {
            debug_print("Too many CNCX records (%u)\n", cncx_records_count);
            mobi_buffer_free_null(buf);
            return MOBI_DATA_CORRUPT;
        }
        /* 56: unk count */
        /* 60-148: phonetizer */
        uint32_t ordt_type = 0;
        uint32_t ordt_entries_count = 0;
        uint32_t ordt1_offset = 0;
        uint32_t ordt2_offset = 0;
        uint32_t index_name_offset = 0;
        uint32_t index_name_length = 0;
        if (header_length >= 180) {
            mobi_buffer_setpos(buf, 164);
            ordt_type = mobi_buffer_get32(buf); /* 164: ORDT type */
            ordt_entries_count = mobi_buffer_get32(buf); /* 168: ORDT entries count */
            ordt1_offset = mobi_buffer_get32(buf); /* 172: ORDT1 offset; currently not used */
            ordt2_offset = mobi_buffer_get32(buf); /* 176: ORDT2 offset */
            const size_t entry_size = (ordt_type == 0) ? 1 : 2;
            if (ordt1_offset + entry_size * ordt_entries_count > indx_record->size
                || ordt2_offset + 2 * ordt_entries_count > indx_record->size) {
                ordt1_offset = 0;
                ordt2_offset = 0;
                ordt_entries_count = 0;
            }
            index_name_offset = mobi_buffer_get32(buf); /* 180: Index name offset */
            index_name_length = mobi_buffer_get32(buf); /* 184: Index name length */
        }
        buf->maxlen = indx_record->size;
        mobi_buffer_setpos(buf, header_length);
        ret = mobi_parse_tagx(buf, tagx);
        if (ret != MOBI_SUCCESS) {
            mobi_buffer_free_null(buf);
            return ret;
        }
        if (ordt_entries_count > 0) {
            /* parse ORDT sections */
            ordt->offsets_count = ordt_entries_count;
            ordt->type = ordt_type;
            ordt->ordt1_pos = ordt1_offset;
            ordt->ordt2_pos = ordt2_offset;
            ret = mobi_parse_ordt(buf, ordt);
            debug_print("ORDT: %u, %u, %u, %u\n", ordt_type, ordt_entries_count, ordt1_offset, ordt2_offset);
            if (ret != MOBI_SUCCESS) {
                mobi_buffer_free_null(buf);
                return ret;
            }
        }
        if (index_name_offset > 0 && index_name_length > 0) {
            if (index_name_length <= header_length - index_name_offset && index_name_length < INDX_NAME_SIZEMAX) {
                mobi_buffer_setpos(buf, index_name_offset);
                char *name = malloc(index_name_length + 1);
                if (name == NULL) {
                    debug_print("%s", "Memory allocation failed\n");
                    mobi_buffer_free_null(buf);
                    return MOBI_MALLOC_FAILED;
                }
                mobi_buffer_getstring(name, buf, index_name_length);
                indx->orth_index_name = name;
                debug_print("Orth index name: %s\n", name);
            }
        }
        indx->encoding = encoding;
        indx->type = type;
        indx->entries_count = entries_count;
        indx->total_entries_count = total_entries_count;
        if (ligt_entries_count != 0 && !mobi_buffer_match_magic_offset(buf, LIGT_MAGIC, ligt_offset)) {
            ligt_offset = 0;
            ligt_entries_count = 0;
        }
        indx->ligt_offset = ligt_offset;
        indx->ligt_entries_count = ligt_entries_count;
        indx->ordt_offset = ordt_offset;
        indx->cncx_records_count = cncx_records_count;
    } else {
        /* else parse IDXT entries offsets */
        if (idxt_offset == 0) {
            debug_print("%s", "Missing IDXT offset\n");
            mobi_buffer_free_null(buf);
            return MOBI_DATA_CORRUPT;
        }
        if (idxt_offset + 2 * entries_count + 4 > indx_record->size ) {
            debug_print("IDXT entries beyond record end%s", "\n");
            mobi_buffer_free_null(buf);
            return MOBI_DATA_CORRUPT;
        }
        mobi_buffer_setpos(buf, idxt_offset);
        MOBIIdxt idxt;
        uint32_t *offsets = malloc((entries_count + 1) * sizeof(uint32_t));
        if (offsets == NULL) {
            mobi_buffer_free_null(buf);
            debug_print("%s\n", "Memory allocation failed");
            return MOBI_MALLOC_FAILED;
        }
        idxt.offsets = offsets;
        ret = mobi_parse_idxt(buf, &idxt, entries_count);
        if (ret != MOBI_SUCCESS) {
            debug_print("%s", "IDXT parsing failed\n");
            mobi_buffer_free_null(buf);
            free(offsets);
            return ret;
        }
        /* parse entries */
        if (entries_count > 0) {
            if (indx->entries == NULL) {
                indx->entries = malloc(indx->total_entries_count * sizeof(MOBIIndexEntry));
                if (indx->entries == NULL) {
                    mobi_buffer_free_null(buf);
                    free(offsets);
                    debug_print("%s\n", "Memory allocation failed");
                    return MOBI_MALLOC_FAILED;
                }
            }
            size_t i = 0;
            while (i < entries_count) {
                ret = mobi_parse_index_entry(indx, idxt, tagx, ordt, buf, i++);
                if (ret != MOBI_SUCCESS) {
                    mobi_buffer_free_null(buf);
                    free(offsets);
                    return ret;
                }
            }
            indx->entries_count += entries_count;
        }
        free(offsets);
    }
    mobi_buffer_free_null(buf);
    return MOBI_SUCCESS;
}