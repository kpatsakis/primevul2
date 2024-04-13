cms_context_fini(cms_context *cms)
{
	struct list_head *n, *pos;

	if (cms->cert) {
		CERT_DestroyCertificate(cms->cert);
		cms->cert = NULL;
	}

	switch (cms->pwdata.source) {
	case PW_SOURCE_INVALID:
	case PW_PROMPT:
	case PW_DEVICE:
	case PW_FROMFILEDB:
	case PW_FROMENV:
	case PW_FROMFILE:
	case PW_FROMFD:
	case PW_SOURCE_MAX:
		break;
	case PW_DATABASE:
		xfree(cms->pwdata.data);
		break;
	case PW_PLAINTEXT:
		memset(cms->pwdata.data, 0, strlen(cms->pwdata.data));
		xfree(cms->pwdata.data);
		break;
	}
	cms->pwdata.source = PW_SOURCE_INVALID;
	cms->pwdata.orig_source = PW_SOURCE_INVALID;

	if (cms->privkey) {
		free(cms->privkey);
		cms->privkey = NULL;
	}

	if (cms->db_out >= 0)
		fsync(cms->db_out);
	xclose(cms->db_out);
	if (cms->dbx_out >= 0)
		fsync(cms->dbx_out);
	xclose(cms->dbx_out);
	if (cms->dbt_out >= 0)
		fsync(cms->dbt_out);
	xclose(cms->dbt_out);
	list_for_each_safe(pos, n, &cms->pk12_ins) {
		pk12_file_t *file = list_entry(pos, pk12_file_t, list);

		xfree(file->path);
		if (file->fd >= 0) {
			/*
			 * This may or may not be writable...
			 */
			fsync(file->fd);
			errno = 0;
		}
		xclose(file->fd);
		xfree(file->pw);
	}
	xclose(cms->pk12_out.fd);
	xfree(cms->pk12_out.path);
	xfree(cms->pk12_out.pw);

	/* These were freed when the arena was destroyed */
	if (cms->tokenname)
		cms->tokenname = NULL;
	if (cms->certname)
		cms->certname = NULL;

	if (cms->newsig.data) {
		free_poison(cms->newsig.data, cms->newsig.len);
		free(cms->newsig.data);
		memset(&cms->newsig, '\0', sizeof (cms->newsig));
	}

	cms->selected_digest = -1;

	if (cms->ci_digest) {
		free_poison(cms->ci_digest->data, cms->ci_digest->len);
		/* XXX sure seems like we should be freeing it here, but
		 * that's segfaulting, and we know it'll get cleaned up with
		 * PORT_FreeArena a couple of lines down.
		 */
		cms->ci_digest = NULL;
	}

	teardown_digests(cms);

	if (cms->raw_signed_attrs) {
		free_poison(cms->raw_signed_attrs->data,
				cms->raw_signed_attrs->len);
		/* XXX sure seems like we should be freeing it here, but
		 * that's segfaulting, and we know it'll get cleaned up with
		 * PORT_FreeArena a couple of lines down.
		 */
		cms->raw_signed_attrs = NULL;
	}

	if (cms->raw_signature) {
		free_poison(cms->raw_signature->data,
				cms->raw_signature->len);
		/* XXX sure seems like we should be freeing it here, but
		 * that's segfaulting, and we know it'll get cleaned up with
		 * PORT_FreeArena a couple of lines down.
		 */
		cms->raw_signature = NULL;
	}

	for (int i = 0; i < cms->num_signatures; i++) {
		free(cms->signatures[i]->data);
		free(cms->signatures[i]);
	}

	xfree(cms->signatures);
	cms->num_signatures = 0;

	if (cms->authbuf) {
		xfree(cms->authbuf);
		cms->authbuf_len = 0;
	}

	PORT_FreeArena(cms->arena, PR_TRUE);
	memset(cms, '\0', sizeof(*cms));
	xfree(cms);
}