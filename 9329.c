match_subject(CERTCertificate *cert, void *cbdatap)
{
	if (!cert->subjectName)
		return 0;

	if (!strcmp(cert->subjectName, (char *)cbdatap))
		return 1;

	return 0;
}