find_named_certificate(cms_context *cms, char *name, CERTCertificate **cert)
{
	if (!name)
		cnreterr(-1, cms, "no subject name specified");

	return find_certificate_by_callback(cms, match_subject, name, cert);
}