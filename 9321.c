cms_context_alloc(cms_context **cmsp)
{
	cms_context *cms = calloc(1, sizeof (*cms));
	if (!cms)
		return -1;

	int rc = cms_context_init(cms);
	if (rc < 0) {
		set_errno_guard();
		xfree(cms);
		cms = NULL;
		return -1;
	}
	*cmsp = cms;
	return 0;
}