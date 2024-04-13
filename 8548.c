njs_string_split_part_add(njs_vm_t *vm, njs_array_t *array, njs_utf8_t utf8,
    const u_char *start, size_t size)
{
    ssize_t  length;

    length = njs_string_calc_length(utf8, start, size);

    return njs_array_string_add(vm, array, start, size, length);
}