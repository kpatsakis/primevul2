static RList *oneshotall_buffer(RBin *bin, RBuffer *b) {
	RBinXtrData *meta = get_the_meta (bin, b);
	if (!meta) {
		return NULL;
	}
	RList *list = r_list_newf (free);
	if (!list) {
		free (meta);
		return NULL;
	}
	r_list_append (list, meta);
	return list;
}