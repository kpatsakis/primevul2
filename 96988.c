iakerb_gss_verify_mic_iov(OM_uint32 *minor_status, gss_ctx_id_t context_handle,
                          gss_qop_t *qop_state, gss_iov_buffer_desc *iov,
                          int iov_count)
{
    iakerb_ctx_id_t ctx = (iakerb_ctx_id_t)context_handle;

    if (ctx->gssc == GSS_C_NO_CONTEXT)
        return GSS_S_NO_CONTEXT;

    return krb5_gss_verify_mic_iov(minor_status, ctx->gssc, qop_state, iov,
                                   iov_count);
}
