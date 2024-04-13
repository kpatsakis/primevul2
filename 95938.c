cifs_parse_smb_version(char *value, struct smb_vol *vol)
{
	substring_t args[MAX_OPT_ARGS];

	switch (match_token(value, cifs_smb_version_tokens, args)) {
	case Smb_1:
		vol->ops = &smb1_operations;
		vol->vals = &smb1_values;
		break;
#ifdef CONFIG_CIFS_SMB2
	case Smb_20:
		vol->ops = &smb21_operations; /* currently identical with 2.1 */
		vol->vals = &smb20_values;
		break;
	case Smb_21:
		vol->ops = &smb21_operations;
		vol->vals = &smb21_values;
		break;
	case Smb_30:
		vol->ops = &smb30_operations;
		vol->vals = &smb30_values;
		break;
#endif
	default:
		cifs_dbg(VFS, "Unknown vers= option specified: %s\n", value);
		return 1;
	}
	return 0;
}
