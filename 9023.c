static char *sqfs_resolve_symlink(struct squashfs_symlink_inode *sym,
				  const char *base_path)
{
	char *resolved, *target;
	u32 sz;

	sz = get_unaligned_le32(&sym->symlink_size);
	target = malloc(sz + 1);
	if (!target)
		return NULL;

	/*
	 * There is no trailling null byte in the symlink's target path, so a
	 * copy is made and a '\0' is added at its end.
	 */
	target[sz] = '\0';
	/* Get target name (relative path) */
	strncpy(target, sym->symlink, sz);

	/* Relative -> absolute path conversion */
	resolved = sqfs_get_abs_path(base_path, target);

	free(target);

	return resolved;
}