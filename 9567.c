update_selected_object_message (gboolean userTriedToSelect)
{
	if (screen.tool != POINTER)
		return;

	gint selectionLength = selection_length (&screen.selectionInfo);

	if (selectionLength == 0) {
		if (userTriedToSelect) {
			/* Update status bar message to make sure the user
			 * knows about needing to select the layer */
			gchar *str = g_new(gchar, MAX_DISTLEN);
			utf8_strncpy(str,
					_("No object selected. Objects can "
					"only be selected in the active "
					"layer."),
					MAX_DISTLEN - 7);
			utf8_snprintf(screen.statusbar.diststr, MAX_DISTLEN,
					"<b>%s</b>", str);
			g_free(str);
		} else {
			utf8_strncpy(screen.statusbar.diststr,
					_("Click to select objects in the "
					"active layer. Middle click and drag "
					"to pan."),
					MAX_DISTLEN);
		}
	} else {
		utf8_snprintf(screen.statusbar.diststr, MAX_DISTLEN,
				ngettext("%d object are currently selected",
					"%d objects are currently selected",
					selectionLength), selectionLength);
	}

	callbacks_update_statusbar();
}