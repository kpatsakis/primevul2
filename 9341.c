find_certificate(cms_context *cms, int needs_private_key)
{
	char *tokenname = resolve_token_name(cms->tokenname);

	struct validity_cbdata cbd;
	if (!cms->certname || !*cms->certname) {
		cms->log(cms, LOG_ERR, "no certificate name specified");
		return -1;
	}

	dprintf("setting password function to %s", cms->func ? "cms->func" : "SECU_GetModulePassword");
	PK11_SetPasswordFunc(cms->func ? cms->func : SECU_GetModulePassword);

	PK11SlotList *slots = NULL;
	slots = PK11_GetAllTokens(CKM_RSA_PKCS, PR_FALSE, PR_TRUE, cms);
	if (!slots)
		cnreterr(-1, cms, "could not get pk11 token list");

	PK11SlotListElement *psle = NULL;
	psle = PK11_GetFirstSafe(slots);
	if (!psle) {
		save_port_err() {
			PK11_FreeSlotList(slots);
		}
		cnreterr(-1, cms, "could not get pk11 safe");
	}

	while (psle) {
		dprintf("looking for token \"%s\", got \"%s\"",
			tokenname, PK11_GetTokenName(psle->slot));
		if (!strcmp(tokenname, PK11_GetTokenName(psle->slot))) {
			dprintf("found token \"%s\"", tokenname);
			break;
		}

		psle = PK11_GetNextSafe(slots, psle, PR_FALSE);
	}

	if (!psle) {
		save_port_err() {
			PK11_FreeSlotList(slots);
		}
		nssreterr(-1, "Could not find token \"%s\"", tokenname);
	}

	int errnum;
	SECStatus status;
	if (PK11_NeedLogin(psle->slot) && !PK11_IsLoggedIn(psle->slot, cms)) {
		status = PK11_Authenticate(psle->slot, PR_TRUE, cms);
		if (status != SECSuccess) {
			save_port_err() {
				errnum = PORT_GetError();
				PK11_DestroySlotListElement(slots, &psle);
				PK11_FreeSlotList(slots);
				cms->log(cms, LOG_ERR,
					 "authentication failed for token \"%s\": %s",
					 tokenname, PORT_ErrorToString(errnum));
			}
			return -1;
		}
	}

	CERTCertList *certlist = NULL;
	certlist = PK11_ListCertsInSlot(psle->slot);
	if (!certlist) {
		save_port_err() {
			PK11_DestroySlotListElement(slots, &psle);
			PK11_FreeSlotList(slots);
		}
		cnreterr(-1, cms, "could not get certificate list");
	}

	SECItem nickname = {
		.data = (void *)cms->certname,
		.len = strlen(cms->certname) + 1,
		.type = siUTF8String,
	};

	cms->psle = psle;

	cbd.cms = cms;
	cbd.psle = psle;
	cbd.slot = psle->slot;
	cbd.cert = NULL;

	PORT_SetError(SEC_ERROR_UNKNOWN_CERT);
	if (needs_private_key) {
		status = PK11_TraverseCertsForNicknameInSlot(&nickname,
					psle->slot, is_valid_cert, &cbd);
		errnum = PORT_GetError();
		if (errnum)
			dprintf("PK11_TraverseCertsForNicknameInSlot():%s:%s",
				PORT_ErrorToName(errnum), PORT_ErrorToString(errnum));
	} else {
		status = PK11_TraverseCertsForNicknameInSlot(&nickname,
					psle->slot,
					is_valid_cert_without_private_key,
					&cbd);
		errnum = PORT_GetError();
		if (errnum)
			dprintf("PK11_TraverseCertsForNicknameInSlot():%s:%s",
				PORT_ErrorToName(errnum), PORT_ErrorToString(errnum));
	}
	dprintf("status:%d cbd.cert:%p", status, cbd.cert);
	if (status == SECSuccess && cbd.cert != NULL) {
		if (cms->cert)
			CERT_DestroyCertificate(cms->cert);
		cms->cert = CERT_DupCertificate(cbd.cert);
	} else {
		errnum = PORT_GetError();
		dprintf("token traversal %s; cert %sfound:%s:%s",
			status == SECSuccess ? "succeeded" : "failed",
			cbd.cert == NULL ? "not" : "",
			PORT_ErrorToName(errnum), PORT_ErrorToString(errnum));
	}

	save_port_err() {
		dprintf("Destroying cert list");
		CERT_DestroyCertList(certlist);
		dprintf("Destroying slot list element");
		PK11_DestroySlotListElement(slots, &psle);
		dprintf("Destroying slot list");
		PK11_FreeSlotList(slots);
		cms->psle = NULL;
	}
	if (status != SECSuccess || cms->cert == NULL)
		cnreterr(-1, cms, "could not find certificate");

	return 0;
}