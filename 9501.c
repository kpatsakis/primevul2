callbacks_toggle_layer_visibility_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	int i = GPOINTER_TO_INT(user_data);

	switch (i) {
	case LAYER_SELECTED:
		i = callbacks_get_selected_row_index ();
		/* No break */
	default:
		if (0 <= i && i <= mainProject->last_loaded) {
			mainProject->file[i]->isVisible = !mainProject->file[i]->isVisible;
		} else {
			/* Not in range */
			return;
		}
		break;
	case LAYER_ALL_ON:
		for (i = 0; i <= mainProject->last_loaded; i++) {
			mainProject->file[i]->isVisible = TRUE;
		}
		break;
	case LAYER_ALL_OFF:
		for (i = 0; i <= mainProject->last_loaded; i++) {
			mainProject->file[i]->isVisible = FALSE;
		}
		break;
	}

	callbacks_update_layer_tree ();

	if (screenRenderInfo.renderType <= GERBV_RENDER_TYPE_GDK_XOR) {
		render_refresh_rendered_image_on_screen ();
	} else {
		render_recreate_composite_surface (screen.drawing_area);
		callbacks_force_expose_event_for_screen ();
	}
}