size_t mobi_trie_get_inflgroups(char **infl_strings, MOBITrie * const root, const char *string) {
    /* travers trie and get values for each substring */
    if (root == NULL) {
        return MOBI_PARAM_ERR;
    }
    size_t count = 0;
    size_t length = strlen(string);
    MOBITrie *node = root;
    while (node && length > 0) {
        char **values = NULL;
        size_t values_count = 0;
        node = mobi_trie_get_next(&values, &values_count, node, string[length - 1]);
        length--;
        for (size_t j = 0; j < values_count; j++) {
            if (count == INDX_INFLSTRINGS_MAX) {
                debug_print("Inflection strings array too small (%d)\n", INDX_INFLSTRINGS_MAX);
                break;
            }
            char infl_string[INDX_LABEL_SIZEMAX + 1];
            const size_t suffix_length = strlen(values[j]);
            if (length + suffix_length > INDX_LABEL_SIZEMAX) {
                debug_print("Label too long (%zu + %zu)\n", length, suffix_length);
                continue;
            }
            memcpy(infl_string, string, length);
            memcpy(infl_string + length, values[j], suffix_length);
            infl_string[length + suffix_length] = '\0';
            infl_strings[count++] = strdup(infl_string);
        }
    }
    return count;
}