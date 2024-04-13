is_valid_cert_without_private_key(CERTCertificate *cert, void *data)
{
	struct validity_cbdata *cbd = (struct validity_cbdata *)data;
	PK11SlotInfo *slot = cbd->slot;
	SECKEYPrivateKey *privkey = NULL;
	int errnum;

	errnum = PORT_GetError();
	if (errnum == SEC_ERROR_EXTENSION_NOT_FOUND) {
		dprintf("Got SEC_ERROR_EXTENSION_NOT_FOUND; clearing");
		PORT_SetError(0);
		errnum = 0;
	}
	if (cert == NULL) {
		if (!errnum)
			PORT_SetError(SEC_ERROR_UNKNOWN_CERT);
		return SECFailure;
	}

	privkey = PK11_FindPrivateKeyFromCert(slot, cert, cbd->cms);
	if (privkey == NULL) {
		if (cbd->cert)
			CERT_DestroyCertificate(cbd->cert);
		PORT_SetError(0);
		cbd->cert = CERT_DupCertificate(cert);
		CERT_DestroyCertificate(cert);
		return SECSuccess;
	} else {
		SECKEY_DestroyPrivateKey(privkey);
		CERT_DestroyCertificate(cert);
	}
	return SECFailure;
}