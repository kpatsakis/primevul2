digest_get_digest_oid(cms_context *cms)
{
	int i = cms->selected_digest;
	return digest_params[i].digest_tag;
}