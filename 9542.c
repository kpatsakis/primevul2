callbacks_reduce_object_area_clicked  (GtkButton *button, gpointer user_data){
	/* for testing, just hard code in some parameters */
	gerbv_image_reduce_area_of_selected_objects (screen.selectionInfo.selectedNodeArray, 0.20, 3, 3, 0.01);
	selection_clear (&screen.selectionInfo);
	update_selected_object_message (FALSE);
	render_refresh_rendered_image_on_screen ();
}