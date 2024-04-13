find_certificate_by_issuer_and_sn(cms_context *cms,
				  CERTIssuerAndSN *ias,
				  CERTCertificate **cert)
{
	if (!ias)
		cnreterr(-1, cms, "invalid issuer and serial number");

	return find_certificate_by_callback(cms, match_issuer_and_serial, &ias, cert);
}