static gint64 cosine_seek_next_packet(wtap *wth, int *err, gchar **err_info,
	char *hdr)
{
	gint64 cur_off;
	char buf[COSINE_LINE_LENGTH];

	while (1) {
		cur_off = file_tell(wth->fh);
		if (cur_off == -1) {
			/* Error */
			*err = file_error(wth->fh, err_info);
			return -1;
		}
		if (file_gets(buf, sizeof(buf), wth->fh) == NULL) {
			*err = file_error(wth->fh, err_info);
			return -1;
		}
		if (strstr(buf, COSINE_REC_MAGIC_STR1) ||
		    strstr(buf, COSINE_REC_MAGIC_STR2)) {
			g_strlcpy(hdr, buf, COSINE_LINE_LENGTH);
			return cur_off;
		}
	}
	return -1;
}
