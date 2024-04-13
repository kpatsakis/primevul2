static int ciedefcompareproc(i_ctx_t *i_ctx_p, ref *space, ref *testspace)
{
    int code = 0;
    ref CIEdict1, CIEdict2;

    code = array_get(imemory, space, 1, &CIEdict1);
    if (code < 0)
        return 0;
    code = array_get(imemory, testspace, 1, &CIEdict2);
    if (code < 0)
        return 0;
    if (!comparedictkey(i_ctx_p, &CIEdict1, &CIEdict2, (char *)"WhitePoint"))
        return 0;
    if (!comparedictkey(i_ctx_p, &CIEdict1, &CIEdict2, (char *)"BlackPoint"))
        return 0;
    if (!comparedictkey(i_ctx_p, &CIEdict1, &CIEdict2, (char *)"RangeABC"))
        return 0;
    if (!comparedictkey(i_ctx_p, &CIEdict1, &CIEdict2, (char *)"DecodeABC"))
        return 0;
    if (!comparedictkey(i_ctx_p, &CIEdict1, &CIEdict2, (char *)"MatrixABC"))
        return 0;
    if (!comparedictkey(i_ctx_p, &CIEdict1, &CIEdict2, (char *)"RangeLMN"))
        return 0;
    if (!comparedictkey(i_ctx_p, &CIEdict1, &CIEdict2, (char *)"DecodeLMN"))
        return 0;
    if (!comparedictkey(i_ctx_p, &CIEdict1, &CIEdict2, (char *)"MatrixMN"))
        return 0;
    if (!comparedictkey(i_ctx_p, &CIEdict1, &CIEdict2, (char *)"RangeDEF"))
        return 0;
    if (!comparedictkey(i_ctx_p, &CIEdict1, &CIEdict2, (char *)"DecodeDEF"))
        return 0;
    if (!comparedictkey(i_ctx_p, &CIEdict1, &CIEdict2, (char *)"RangeHIJ"))
        return 0;
    if (!comparedictkey(i_ctx_p, &CIEdict1, &CIEdict2, (char *)"Table"))
        return 0;
    return 1;
}
