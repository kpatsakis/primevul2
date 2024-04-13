int CMS_decrypt(CMS_ContentInfo *cms, EVP_PKEY *pk, X509 *cert,
                BIO *dcont, BIO *out, unsigned int flags)
{
    int r;
    BIO *cont;
    if (OBJ_obj2nid(CMS_get0_type(cms)) != NID_pkcs7_enveloped) {
        CMSerr(CMS_F_CMS_DECRYPT, CMS_R_TYPE_NOT_ENVELOPED_DATA);
        return 0;
    }
    if (!dcont && !check_content(cms))
        return 0;
    if (flags & CMS_DEBUG_DECRYPT)
         cms->d.envelopedData->encryptedContentInfo->debug = 1;
     else
         cms->d.envelopedData->encryptedContentInfo->debug = 0;
    if (!cert)
        cms->d.envelopedData->encryptedContentInfo->havenocert = 1;
    else
        cms->d.envelopedData->encryptedContentInfo->havenocert = 0;
     if (!pk && !cert && !dcont && !out)
         return 1;
     if (pk && !CMS_decrypt_set1_pkey(cms, pk, cert))
    r = cms_copy_content(out, cont, flags);
    do_free_upto(cont, dcont);
    return r;
}
