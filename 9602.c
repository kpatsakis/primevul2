callbacks_layer_tree_button_press (GtkWidget *widget, GdkEventButton *event,
					gpointer user_data)
{
	GtkTreePath *path;
	GtkTreeIter iter;
	GtkTreeViewColumn *col;
	gint x,y;
	gint *indices;

	GtkListStore *list_store = (GtkListStore *) gtk_tree_view_get_model (
			(GtkTreeView *) screen.win.layerTree);

	if (event->button == 1) {
		if (gtk_tree_view_get_path_at_pos ((GtkTreeView *) widget,
				event->x, event->y, &path, &col, &x, &y)
		&& gtk_tree_model_get_iter ((GtkTreeModel *)list_store,
				&iter, path)) {
			indices = gtk_tree_path_get_indices (path);
			if (indices && (indices[0] <= mainProject->last_loaded)) {
				switch (callbacks_get_col_num_from_tree_view_col (col)) {
				case 0:
					callbacks_select_layer_row (indices[0]);
					callbacks_layer_tree_visibility_toggled (indices[0]);
					return TRUE;
				case 1:
					callbacks_show_color_picker_dialog (indices[0]);
					/* don't propagate the signal, since drag and drop can
					   sometimes activated during color selection */
					return TRUE;
				}
			}
		}
	}
	/* don't pop up the menu if we don't have any loaded files */
	else if ((event->button == 3)&&(mainProject->last_loaded >= 0)) {
		gtk_menu_popup (GTK_MENU (screen.win.layerTreePopupMenu),
				NULL, NULL, NULL, NULL, 
				event->button, event->time);
	}

	/* always allow the click to propagate and make sure the line is activated */
	return FALSE;
}