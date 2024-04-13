static int validatecalgrayspace(i_ctx_t * i_ctx_p, ref **r)
{
    int code=0;
    ref *space, calgraydict;

    space = *r;
    if (!r_is_array(space))
        return_error(gs_error_typecheck);
    /* Validate parameters, check we have enough operands */
    if (r_size(space) < 2)
        return_error(gs_error_rangecheck);
    code = array_get(imemory, space, 1, &calgraydict);
    if (code < 0)
        return code;
    check_type(calgraydict, t_dictionary);
    /* Check the white point, which is required */
    /* We have to have a white point */
    /* Check white point exists, and is an array of three numbers */
    code = checkWhitePoint(i_ctx_p, &calgraydict);
    if (code != 0)
        return code;
    /* The rest are optional.  Need to validate though */
    code = checkBlackPoint(i_ctx_p, &calgraydict);
    if (code < 0)
        return code;
    /* Check Gamma values */
    code = checkGamma(i_ctx_p, &calgraydict, 1);
    if (code < 0)
        return code;
    *r = 0;  /* No nested space */
    return 0;
}
