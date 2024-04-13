callbacks_select_layer_row (gint rowIndex)
{
	GtkTreeSelection *selection;
	GtkTreeIter iter;
	GtkListStore *list_store = (GtkListStore *) gtk_tree_view_get_model
			((GtkTreeView *) screen.win.layerTree);

	selection = gtk_tree_view_get_selection((GtkTreeView *) screen.win.layerTree);

	if (gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(list_store),
				&iter, NULL, rowIndex)) {
		gtk_tree_selection_select_iter (selection, &iter);
	}
}