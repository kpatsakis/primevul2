callbacks_change_layer_color_clicked  (GtkButton *button, gpointer   user_data) {
	gint index=callbacks_get_selected_row_index();
	if (index < 0) {
		show_no_layers_warning ();
		return;
	}
	callbacks_show_color_picker_dialog (index);
}