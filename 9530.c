callbacks_layer_tree_key_press (GtkWidget *widget, GdkEventKey *event, gpointer user_data) {
	/* if space is pressed while a color picker icon is in focus,
	show the color picker dialog. */

	if(event->keyval == GDK_space){
		GtkTreeView *tree;
		GtkTreePath *path;
		GtkTreeViewColumn *col;
		gint *indices;
		gint idx;

		tree = (GtkTreeView *) screen.win.layerTree;
		gtk_tree_view_get_cursor (tree, &path, &col);
		if (path) {
			indices = gtk_tree_path_get_indices (path);
			if (indices) {
				idx = callbacks_get_col_num_from_tree_view_col (col);
				if ((idx == 1) && (indices[0] <= mainProject->last_loaded)){
					callbacks_show_color_picker_dialog (indices[0]);
				}
			}
			gtk_tree_path_free (path);
		}
	}
	/* by default propagate the key press */
	return FALSE;
}