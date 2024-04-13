static int x509_get_version( unsigned char **p,
                             const unsigned char *end,
                             int *ver )
{
    int ret;
    size_t len;

    if( ( ret = mbedtls_asn1_get_tag( p, end, &len,
            MBEDTLS_ASN1_CONTEXT_SPECIFIC | MBEDTLS_ASN1_CONSTRUCTED | 0 ) ) != 0 )
    {
        if( ret == MBEDTLS_ERR_ASN1_UNEXPECTED_TAG )
        {
            *ver = 0;
            return( 0 );
        }

        return( ret );
    }

    end = *p + len;

    if( ( ret = mbedtls_asn1_get_int( p, end, ver ) ) != 0 )
        return( MBEDTLS_ERR_X509_INVALID_VERSION + ret );

    if( *p != end )
        return( MBEDTLS_ERR_X509_INVALID_VERSION +
                MBEDTLS_ERR_ASN1_LENGTH_MISMATCH );

    return( 0 );
}
