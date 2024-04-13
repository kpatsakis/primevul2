digest_get_encryption_oid(cms_context *cms)
{
	int i = cms->selected_digest;
	return digest_params[i].digest_encryption_tag;
}