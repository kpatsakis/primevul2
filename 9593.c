callbacks_move_layer_up_button_clicked  (GtkButton *button, gpointer   user_data) {
	gint index=callbacks_get_selected_row_index();
	if (index < 0) {
		show_no_layers_warning ();
		return;
	}
	if (index > 0) {
		gerbv_change_layer_order (mainProject, index, index - 1);
		callbacks_update_layer_tree ();
		callbacks_select_layer_row (index - 1);
		if (screenRenderInfo.renderType <= GERBV_RENDER_TYPE_GDK_XOR) {
			render_refresh_rendered_image_on_screen();
		}
		else {
			render_recreate_composite_surface (screen.drawing_area);
			callbacks_force_expose_event_for_screen ();
		}
	}
}