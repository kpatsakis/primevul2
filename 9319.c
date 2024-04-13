generate_signature(cms_context *cms)
{
	int rc = 0;

	if (cms->digests[cms->selected_digest].pe_digest == NULL)
		cnreterr(-1, cms, "PE digest has not been allocated");

	if (content_is_empty(cms->digests[cms->selected_digest].pe_digest->data,
			cms->digests[cms->selected_digest].pe_digest->len))
		cnreterr(-1, cms, "PE binary has not been digested");

	SECItem sd_der;
	memset(&sd_der, '\0', sizeof(sd_der));
	rc = generate_spc_signed_data(cms, &sd_der);
	if (rc < 0)
		cnreterr(-1, cms, "could not create signed data");

	memcpy(&cms->newsig, &sd_der, sizeof (cms->newsig));
	cms->newsig.data = malloc(sd_der.len);
	if (!cms->newsig.data)
		cnreterr(-1, cms, "could not allocate signed data");
	memcpy(cms->newsig.data, sd_der.data, sd_der.len);
	return 0;
}