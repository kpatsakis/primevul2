njs_string_prop(njs_string_prop_t *string, const njs_value_t *value)
{
    size_t     size;
    uintptr_t  length;

    size = value->short_string.size;

    if (size != NJS_STRING_LONG) {
        string->start = (u_char *) value->short_string.start;
        length = value->short_string.length;

    } else {
        string->start = (u_char *) value->long_string.data->start;
        size = value->long_string.size;
        length = value->long_string.data->length;
    }

    string->size = size;
    string->length = length;

    return (length == 0) ? size : length;
}