static int x509_profile_check_pk_alg( const mbedtls_x509_crt_profile *profile,
                                      mbedtls_pk_type_t pk_alg )
{
    if( ( profile->allowed_pks & MBEDTLS_X509_ID_FLAG( pk_alg ) ) != 0 )
        return( 0 );

    return( -1 );
}
