find_certificate_by_callback(cms_context *cms,
			     find_cert_match_t *match, void *cbdata,
			     CERTCertificate **cert)
{
	char *tokenname = resolve_token_name(cms->tokenname);

	if (!match) {
		cms->log(cms, LOG_ERR, "no certificate match callback not specified");
		return -1;
	}
	if (!cbdata) {
		cms->log(cms, LOG_ERR, "no certificate callback data not specified");
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
		if (!strcmp(tokenname, PK11_GetTokenName(psle->slot)))
			break;

		psle = PK11_GetNextSafe(slots, psle, PR_FALSE);
	}

	if (!psle) {
		save_port_err() {
			PK11_FreeSlotList(slots);
			cms->log(cms, LOG_ERR, "could not find token \"%s\"",
				 tokenname);
		}
		return -1;
	}

	SECStatus status;
	if (PK11_NeedLogin(psle->slot) && !PK11_IsLoggedIn(psle->slot, cms)) {
		status = PK11_Authenticate(psle->slot, PR_TRUE, cms);
		if (status != SECSuccess) {
			save_port_err() {
				int err = PORT_GetError();
				PK11_DestroySlotListElement(slots, &psle);
				PK11_FreeSlotList(slots);
				cms->log(cms, LOG_ERR,
					 "authentication failed for token \"%s\": %s",
					 tokenname, PORT_ErrorToString(err));
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

	CERTCertListNode *node = NULL;
	for_each_cert(certlist, tmpnode) {
		/* If we're looking up the issuer of some cert, and the
		 * issuer isn't in the database, we'll get back what is
		 * essentially a template that's in NSS's cache waiting to
		 * be filled out.  We can't use that, it'll just cause
		 * CERT_DupCertificate() to segfault. */
		if (!tmpnode || !tmpnode->cert
		    || !tmpnode->cert->derCert.data
		    || !tmpnode->cert->derCert.len
		    || !tmpnode->cert->derIssuer.data
		    || !tmpnode->cert->derIssuer.len
		    || !tmpnode->cert->serialNumber.data
		    || !tmpnode->cert->serialNumber.len)
			continue;

		int rc = match(tmpnode->cert, cbdata);
		if (rc == 0) {
			node = tmpnode;
			break;
		}
	}

	if (!node)
		cnreterr(-1, cms, "Could not find certificate");

	*cert = CERT_DupCertificate(node->cert);

	PK11_DestroySlotListElement(slots, &psle);
	PK11_FreeSlotList(slots);
	CERT_DestroyCertList(certlist);

	return 0;

}