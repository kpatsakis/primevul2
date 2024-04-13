callbacks_get_col_num_from_tree_view_col (GtkTreeViewColumn *col)
{
	GList *cols;
	gint   num;
	
	g_return_val_if_fail ( col != NULL, -1 );
	g_return_val_if_fail ( col->tree_view != NULL, -1 );
	cols = gtk_tree_view_get_columns(GTK_TREE_VIEW(col->tree_view));
	num = g_list_index(cols, (gpointer) col);
	g_list_free(cols);
	return num;
}