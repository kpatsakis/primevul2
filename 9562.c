callbacks_update_layer_tree (void)
{
	GtkListStore *list_store = (GtkListStore *) gtk_tree_view_get_model
			((GtkTreeView *) screen.win.layerTree);
	gint idx;
	GtkTreeIter iter;
	GtkTreeSelection *selection;
	gint oldSelectedRow;

	if (screen.win.treeIsUpdating)
		return;

	screen.win.treeIsUpdating = TRUE;

	oldSelectedRow = callbacks_get_selected_row_index();
	if (oldSelectedRow < 0)
		oldSelectedRow = 0;

	gtk_list_store_clear (list_store);

	for (idx = 0; idx <= mainProject->last_loaded; idx++) {
		GdkPixbuf *pixbuf, *blackPixbuf;
		unsigned char red, green, blue, alpha;
		guint32 color;
		gchar *layerName;
		gerbv_fileinfo_t *file;
		
		file = mainProject->file[idx];
		if (!file)
			continue;

		red =   (unsigned char) (file->color.red * 255 / G_MAXUINT16);
		green = (unsigned char) (file->color.green * 255 / G_MAXUINT16);
		blue =  (unsigned char) (file->color.blue *255 / G_MAXUINT16);
		alpha = (unsigned char) (file->alpha * 255 / G_MAXUINT16);

		color = red*(256*256*256) + green*(256*256) + blue*256 + alpha;
		pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB, TRUE, 8, 20, 15);
		gdk_pixbuf_fill (pixbuf, color);

		blackPixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB, TRUE, 8, 20, 15);
		color = 100*(256*256*256) + 100*(256*256) + 100*256 + 150;
		gdk_pixbuf_fill (blackPixbuf, color);

		/* copy the color area into the black pixbuf */
		gdk_pixbuf_copy_area  (pixbuf, 1, 1, 18, 13, blackPixbuf, 1, 1);
		g_object_unref(pixbuf);

		gtk_list_store_append (list_store, &iter);

		gchar startChar[2],*modifiedCode;
		/* terminate the letter string */
		startChar[1] = 0;

		gint numberOfModifications = 0;
		if (file->transform.inverted) {
			startChar[0] = 'I';
			numberOfModifications++;
		}
		if (file->transform.mirrorAroundX
		||  file->transform.mirrorAroundY) {
			startChar[0] = 'M';
			numberOfModifications++;
		}
		if (fabs(file->transform.translateX)
					> GERBV_PRECISION_LINEAR_INCH
		||  fabs(file->transform.translateY)
					> GERBV_PRECISION_LINEAR_INCH) {
			startChar[0] = 'T';
			numberOfModifications++;
		}
		if (fabs(file->transform.scaleX - 1)
					> GERBV_PRECISION_LINEAR_INCH
		||  fabs(file->transform.scaleY - 1)
					> GERBV_PRECISION_LINEAR_INCH) {
			startChar[0] = 'S';
			numberOfModifications++;
		}
		if (fabs(file->transform.rotation)
					> GERBV_PRECISION_ANGLE_RAD) {
			startChar[0] = 'R';
			numberOfModifications++;
		}
		if (numberOfModifications > 1)
			startChar[0] = '*';
		if (numberOfModifications == 0)
			modifiedCode = g_strdup ("");
		else
			modifiedCode = g_strdup (startChar);

		/* Display any unsaved layers differently to show the user they
		 * are unsaved */
		if (file->layer_dirty == TRUE) {
			/* The layer has unsaved changes in it. Show layer name
			 * in italics. */
			layerName = g_strconcat ("*", "<i>", file->name,
					"</i>", NULL);
		} else {
			/* Layer is clean. Show layer name using normal font. */
			layerName = g_strdup (file->name);
		}

		gtk_list_store_set (list_store, &iter,
				    0, file->isVisible,
				    1, blackPixbuf,
				    2, layerName,
				    3, modifiedCode,
				    -1);
		g_free (layerName);
		g_free (modifiedCode);
		/* pixbuf has a refcount of 2 now, as the list store has added its own reference */
		g_object_unref(blackPixbuf);
	}

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(screen.win.layerTree));

	/* if no line is selected yet, select the first row (if it has data) */
	/* or, select the line that was previously selected */

	if (!gtk_tree_selection_get_selected (selection, NULL, &iter)) {
		if (gtk_tree_model_iter_nth_child ((GtkTreeModel *) list_store,
						&iter, NULL, oldSelectedRow)) {
			gtk_tree_selection_select_iter (selection, &iter);
		}
	}
	gboolean showItems = (mainProject->last_loaded >= 0);
	gtk_widget_set_sensitive (screen.win.curLayerMenuItem, showItems);
	gtk_widget_set_sensitive (screen.win.curAnalyzeMenuItem, showItems);
	gtk_widget_set_sensitive (screen.win.curEditMenuItem, showItems);
	for (unsigned int i = 0;
			i < G_N_ELEMENTS(screen.win.curFileMenuItem); i++) {
		gtk_widget_set_sensitive (screen.win.curFileMenuItem[i], showItems);
	}
	screen.win.treeIsUpdating = FALSE;
}