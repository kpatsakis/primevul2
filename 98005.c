int mbedtls_x509_crt_verify( mbedtls_x509_crt *crt,
                     mbedtls_x509_crt *trust_ca,
                     mbedtls_x509_crl *ca_crl,
                     const char *cn, uint32_t *flags,
                     int (*f_vrfy)(void *, mbedtls_x509_crt *, int, uint32_t *),
                     void *p_vrfy )
{
    return( mbedtls_x509_crt_verify_with_profile( crt, trust_ca, ca_crl,
                &mbedtls_x509_crt_profile_default, cn, flags, f_vrfy, p_vrfy ) );
}
