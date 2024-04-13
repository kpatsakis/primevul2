callbacks_remove_layer_button_clicked (GtkButton *button, gpointer user_data)
{
	gint index = callbacks_get_selected_row_index ();
	
	if ((index >= 0) && (index <= mainProject->last_loaded)) {
		render_remove_selected_objects_belonging_to_layer (
				&screen.selectionInfo,
				mainProject->file[index]->image);
		update_selected_object_message (FALSE);

		gerbv_unload_layer (mainProject, index);
		callbacks_update_layer_tree ();

		index = MAX(0, index - 1);
		callbacks_select_layer_row (index);

		if (screenRenderInfo.renderType <= GERBV_RENDER_TYPE_GDK_XOR) {
			render_refresh_rendered_image_on_screen ();
		} else {
			render_recreate_composite_surface (screen.drawing_area);
			callbacks_force_expose_event_for_screen ();
		}
	}
}