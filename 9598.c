callbacks_file_drop_event(GtkWidget *widget, GdkDragContext *dc,
		gint x, gint y, GtkSelectionData *data,
		guint info, guint time, gpointer p)
{
	gchar **uris, **uri;
	GSList *fns = NULL;

	uris = gtk_selection_data_get_uris(data);
	if (!uris)
		return FALSE;

	for (uri = uris; *uri; uri++) {
		const char *prefix_str =
#ifdef WIN32
			"file:///";
#else
			"file://";
#endif
		if (g_strrstr(*uri, prefix_str) == *uri)
			fns = g_slist_append(fns, g_uri_unescape_string(
					*uri + strlen(prefix_str), NULL));
	}

	open_files(fns);
	g_slist_free_full(fns, g_free);
	g_strfreev(uris);

	return TRUE;
}