static char *sqfs_get_abs_path(const char *base, const char *rel)
{
	char **base_tokens, **rel_tokens, *resolved = NULL;
	int ret, bc, rc, i, updir = 0, resolved_size = 0, offset = 0;

	base_tokens = NULL;
	rel_tokens = NULL;

	/* Memory allocation for the token lists */
	bc = sqfs_count_tokens(base);
	rc = sqfs_count_tokens(rel);
	if (bc < 1 || rc < 1)
		return NULL;

	base_tokens = calloc(bc, sizeof(char *));
	if (!base_tokens)
		return NULL;

	rel_tokens = calloc(rc, sizeof(char *));
	if (!rel_tokens)
		goto out;

	/* Fill token lists */
	ret = sqfs_tokenize(base_tokens, bc, base);
	if (ret)
		goto out;

	ret = sqfs_tokenize(rel_tokens, rc, rel);
	if (ret)
		goto out;

	/* count '..' occurrences in target path */
	for (i = 0; i < rc; i++) {
		if (!strcmp(rel_tokens[i], ".."))
			updir++;
	}

	/* Remove the last token and the '..' occurrences */
	bc = sqfs_clean_base_path(base_tokens, bc, updir);
	if (bc < 0)
		goto out;

	/* Calculate resolved path size */
	if (!bc)
		resolved_size++;

	resolved_size += sqfs_get_tokens_length(base_tokens, bc) +
		sqfs_get_tokens_length(rel_tokens, rc);

	resolved = malloc(resolved_size + 1);
	if (!resolved)
		goto out;

	/* Set resolved path */
	memset(resolved, '\0', resolved_size + 1);
	offset += sqfs_join(base_tokens, resolved + offset, 0, bc, '/');
	resolved[offset++] = '/';
	offset += sqfs_join(rel_tokens, resolved + offset, updir, rc, '/');

out:
	if (rel_tokens)
		for (i = 0; i < rc; i++)
			free(rel_tokens[i]);
	if (base_tokens)
		for (i = 0; i < bc; i++)
			free(base_tokens[i]);

	free(rel_tokens);
	free(base_tokens);

	return resolved;
}