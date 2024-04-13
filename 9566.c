callbacks_move_objects_clicked (GtkButton *button, gpointer   user_data){
	/* for testing, just hard code in some translations here */
	gerbv_image_move_selected_objects (screen.selectionInfo.selectedNodeArray, -0.050, 0.050);
	callbacks_update_layer_tree();
	selection_clear (&screen.selectionInfo);
	update_selected_object_message (FALSE);
	render_refresh_rendered_image_on_screen ();
}