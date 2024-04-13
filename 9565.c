callbacks_invert_layer_clicked  (GtkButton *button, gpointer   user_data) {
	gint index=callbacks_get_selected_row_index();
	if (index < 0) {
		show_no_layers_warning ();
		return;
	}
	mainProject->file[index]->transform.inverted = !mainProject->file[index]->transform.inverted;
	render_refresh_rendered_image_on_screen ();
	callbacks_update_layer_tree ();
}