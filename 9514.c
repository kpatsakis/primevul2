callbacks_reload_layer_clicked (GtkButton *button, gpointer user_data)
{
	gint index = callbacks_get_selected_row_index ();

	if (index < 0) {
		show_no_layers_warning ();
		return;
	}

	render_remove_selected_objects_belonging_to_layer (
			&screen.selectionInfo, mainProject->file[index]->image);
	update_selected_object_message (FALSE);

	gerbv_revert_file (mainProject, index);
	render_refresh_rendered_image_on_screen ();
	callbacks_update_layer_tree();
}