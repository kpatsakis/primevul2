callbacks_revert_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	gerbv_revert_all_files (mainProject);
	selection_clear (&screen.selectionInfo);
	update_selected_object_message (FALSE);
	render_refresh_rendered_image_on_screen ();
	callbacks_update_layer_tree ();
}