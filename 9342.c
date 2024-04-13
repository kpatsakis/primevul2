digest_get_signature_oid(cms_context *cms)
{
	int i = cms->selected_digest;
	return digest_params[i].signature_tag;
}