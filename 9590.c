callbacks_new_project_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	if (mainProject->last_loaded >= 0) {
		if (!interface_get_alert_dialog_response (
			_("Do you want to close any open layers "
			"and start a new project?"),
			_("Starting a new project will cause all currently "
			"open layers to be closed. Any unsaved changes "
			"will be lost."),
			FALSE, NULL, GTK_STOCK_CLOSE, GTK_STOCK_CANCEL))
			return;
	}
	/* Unload all layers and then clear layer window */
	gerbv_unload_all_layers (mainProject);
	callbacks_update_layer_tree ();
	selection_clear (&screen.selectionInfo);
	update_selected_object_message (FALSE);
	
	/* Destroy project info */
	if (mainProject->project) {
	    g_free(mainProject->project);
	    mainProject->project = NULL;
	}
	render_refresh_rendered_image_on_screen();
}