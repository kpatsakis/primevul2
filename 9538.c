callbacks_get_selected_row_index (void)
{
	GtkTreeSelection *selection;
	GtkTreeIter       iter;
	GtkListStore *list_store = (GtkListStore *) gtk_tree_view_get_model
			((GtkTreeView *) screen.win.layerTree);
	gint index=-1,i=0;

	/* This will only work in single or browse selection mode! */
	selection = gtk_tree_view_get_selection((GtkTreeView *) screen.win.layerTree);
	if (gtk_tree_selection_get_selected(selection, NULL, &iter)) {
		while (gtk_tree_model_iter_nth_child ((GtkTreeModel *)list_store,
				&iter, NULL, i)){
			if (gtk_tree_selection_iter_is_selected (selection, &iter)) {
				return i;
			}
			i++;
     		}
	}
	return index;
}