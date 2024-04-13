njs_string_to_number(const njs_value_t *value, njs_bool_t parse_float)
{
    double                num;
    size_t                size;
    uint32_t              u;
    njs_bool_t            minus;
    const u_char          *p, *start, *end;
    njs_unicode_decode_t  ctx;

    const size_t  infinity = njs_length("Infinity");

    size = value->short_string.size;

    if (size != NJS_STRING_LONG) {
        p = value->short_string.start;

    } else {
        size = value->long_string.size;
        p = value->long_string.data->start;
    }

    end = p + size;

    njs_utf8_decode_init(&ctx);

    while (p < end) {
        start = p;
        u = njs_utf8_decode(&ctx, &p, end);

        if (!njs_utf8_is_whitespace(u)) {
            p = start;
            break;
        }
    }

    if (p == end) {
        return parse_float ? NAN : 0.0;
    }

    minus = 0;

    if (*p == '+') {
        p++;

    } else if (*p == '-') {
        p++;
        minus = 1;
    }

    if (p == end) {
        return NAN;
    }

    if (!parse_float
        && p + 2 < end && p[0] == '0' && (p[1] == 'x' || p[1] == 'X'))
    {
        p += 2;
        num = njs_number_hex_parse(&p, end, 0);

    } else {
        start = p;
        num = njs_number_dec_parse(&p, end, 0);

        if (p == start) {
            if (p + infinity > end || memcmp(p, "Infinity", infinity) != 0) {
                return NAN;
            }

            num = INFINITY;
            p += infinity;
        }
    }

    if (!parse_float) {
        while (p < end) {
            if (*p != ' ' && *p != '\t') {
                return NAN;
            }

            p++;
        }
    }

    return minus ? -num : num;
}