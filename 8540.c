njs_string_to_index(const njs_value_t *value)
{
    size_t        size, len;
    double        num;
    njs_bool_t    minus;
    const u_char  *p, *start, *end;
    u_char        buf[128];

    size = value->short_string.size;

    if (size != NJS_STRING_LONG) {
        start = value->short_string.start;

    } else {
        size = value->long_string.size;
        start = value->long_string.data->start;
    }

    p = start;
    end = p + size;
    minus = 0;

    if (size > 1) {
        switch (p[0]) {
        case '0':
            if (size != 1) {
                return NAN;
            }

            /* Fall through. */

        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            break;

        case '-':
            if (size == 2 && p[1] == '0') {
                return -0.0;
            }

            if (size == njs_length("-Infinity")
                && memcmp(&p[1], "Infinity", njs_length("Infinity")) == 0)
            {
                return -INFINITY;
            }

            p++;
            minus = 1;

            break;

        case 'I':
            if (size == njs_length("Infinity")
                && memcmp(p, "Infinity", njs_length("Infinity")) == 0)
            {
                return INFINITY;
            }

            /* Fall through. */

        default:
            return NAN;
        }
    }

    num = njs_strtod(&p, end, 0);
    if (p != end) {
        return NAN;
    }

    num = minus ? -num : num;

    len = njs_dtoa(num, (char *) buf);
    if (size != len || memcmp(start, buf, size) != 0) {
        return NAN;
    }

    return num;
}