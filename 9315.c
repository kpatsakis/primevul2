is_issuer_of(CERTCertificate *c0, CERTCertificate *c1)
{
	if (c0->derSubject.len != c1->derIssuer.len)
		return 0;

	if (memcmp(c0->derSubject.data, c1->derIssuer.data, c0->derSubject.len))
		return 0;
	return 1;
}