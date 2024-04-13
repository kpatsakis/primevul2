digest_get_digest_size(cms_context *cms)
{
	int i = cms->selected_digest;
	return digest_params[i].size;
}