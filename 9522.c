callbacks_delete_objects_clicked (GtkButton *button, gpointer user_data)
{
	if (selection_length (&screen.selectionInfo) == 0) {
		interface_show_alert_dialog (
			_("No object is currently selected"),
			_("Objects must be selected using the pointer tool "
				"before they can be deleted."),
			FALSE,
			NULL);
		return;
	}

	gint index = callbacks_get_selected_row_index ();
	if (index < 0)
		return;

	if (mainProject->check_before_delete) {
		if (!interface_get_alert_dialog_response (
			_("Do you want to permanently delete "
			"the selected objects from <i>visible</i> layers?"),
			_("Gerbv currently has no undo function, so "
			"this action cannot be undone. This action "
			"will not change the saved file unless you "
			"save the file afterwards."),
			TRUE, &(mainProject->check_before_delete),
			GTK_STOCK_DELETE, GTK_STOCK_CANCEL)) {
				return;
		}
	}

	guint i;
	for (i = 0; i < selection_length (&screen.selectionInfo);) {
		gerbv_selection_item_t sel_item =
			selection_get_item_by_index (&screen.selectionInfo, i);
		gerbv_fileinfo_t *file_info =
			gerbv_get_fileinfo_for_image(sel_item.image, mainProject);

		/* Preserve currently invisible selection from deletion */
		if (!file_info->isVisible) {
			i++;
			continue;
		}

		file_info->layer_dirty = TRUE;
		selection_clear_item_by_index (&screen.selectionInfo, i);
		gerbv_image_delete_net (sel_item.net);
	}
	update_selected_object_message (FALSE);

	render_refresh_rendered_image_on_screen ();
	callbacks_update_layer_tree();
}