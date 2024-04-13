static int ciearange(i_ctx_t * i_ctx_p, ref *space, float *ptr)
{
    int code;
    ref CIEdict, *tempref;

    code = array_get(imemory, space, 1, &CIEdict);
    if (code < 0)
        return code;

    /* If we have a RangeA entry in the dictionary, get the
     * values from that
     */
    code = dict_find_string(&CIEdict, "RangeA", &tempref);
    if (code > 0 && !r_has_type(tempref, t_null)) {
        code = get_cie_param_array(imemory, tempref, 2, ptr);
        if (code < 0)
            return code;
    } else {
        /* Default values for CIEBasedA */
        ptr[0] = 0;
        ptr[1] = 1;
    }
    return 0;
}
