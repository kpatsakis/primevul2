callbacks_change_layer_edit_clicked  (GtkButton *button, gpointer userData)
{
	gint index = callbacks_get_selected_row_index();
	gerbv_fileinfo_t **files = mainProject->file;
	gerbv_user_transformation_t **transforms;
	int i, j;

	if (index < 0) {
		show_no_layers_warning ();
		return;
	}

	/* last_loaded == 0 if only one file is loaded */
	transforms = g_new (gerbv_user_transformation_t *,
			mainProject->last_loaded +
			2 /* layer + NULL */ +
			1 /* if selected layer is visible */);

	/* [0] is selected layer */
	transforms[0] = &mainProject->file[index]->transform;

	/* Get visible Gerber files transformations */
	j = 1;	/* [0] is alerady used */
	for (i = 0; i <= mainProject->last_loaded; i++) {
		if (files[i] && files[i]->isVisible)
			transforms[j++] = &files[i]->transform;
	}

	/* Terminate array with NULL */
	transforms[j] = NULL;

	interface_show_layer_edit_dialog(transforms, screen.unit);
	g_free (transforms);
	render_refresh_rendered_image_on_screen ();
	callbacks_update_layer_tree ();	
}