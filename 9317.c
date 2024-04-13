unlock_nss_token(cms_context *cms)
{
	char *tokenname = resolve_token_name(cms->tokenname);

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
		}
		nssreterr(-1, "Could not find token \"%s\"", tokenname);
	}

	SECStatus status;
	if (PK11_NeedLogin(psle->slot) &&
			!PK11_IsLoggedIn(psle->slot, cms)) {
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

	PK11_DestroySlotListElement(slots, &psle);
	PK11_FreeSlotList(slots);
	return 0;
}