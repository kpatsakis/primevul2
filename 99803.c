struct nfc_llcp_local *nfc_llcp_local_get(struct nfc_llcp_local *local)
{
	kref_get(&local->ref);

	return local;
}