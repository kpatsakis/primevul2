cms_context_init(cms_context *cms)
{
	memset(cms, '\0', sizeof (*cms));

	cms->log = cms_common_log;

	cms->arena = PORT_NewArena(DER_DEFAULT_CHUNKSIZE);
	if (!cms->arena)
		cnreterr(-1, cms, "could not create cryptographic arena");

	cms->selected_digest = -1;

	INIT_LIST_HEAD(&cms->pk12_ins);
	cms->pk12_out.fd = -1;
	cms->db_out = cms->dbx_out = cms->dbt_out = -1;

	return 0;
}