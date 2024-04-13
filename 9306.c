match_issuer_and_serial(CERTCertificate *cert, void *cbdatap)
{
	CERTIssuerAndSN *ias = cbdatap;
	bool found = false;

	if (ias->derIssuer.len == cert->derIssuer.len &&
	    ias->derIssuer.len != 0) {
		if (memcmp(ias->derIssuer.data, cert->derIssuer.data,
			   ias->derIssuer.len))
			return 0;
		found = true;
	}

	if (!found) {
		SECComparison seccomp;

		seccomp = CERT_CompareName(&ias->issuer, &cert->issuer);
		if (seccomp != SECEqual)
			return 0;
	}

	if (ias->serialNumber.len != cert->serialNumber.len)
		return 0;

	if (memcmp(ias->serialNumber.data, cert->serialNumber.data,
		   ias->serialNumber.len))
		return 0;

	return 1;
}