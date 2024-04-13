callbacks_open_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	GSList *fns = NULL;
	screen.win.gerber = 
		gtk_file_chooser_dialog_new (
				_("Open Gerbv project, Gerber, drill, "
				"or pick&place files"),
			NULL, GTK_FILE_CHOOSER_ACTION_OPEN,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_OPEN,   GTK_RESPONSE_ACCEPT,
			NULL);

	gtk_file_chooser_set_select_multiple(
			(GtkFileChooser *)screen.win.gerber, TRUE);
	gtk_file_chooser_set_current_folder(
			(GtkFileChooser *)screen.win.gerber, mainProject->path);
	gtk_widget_show (screen.win.gerber);
	if (gtk_dialog_run ((GtkDialog*)screen.win.gerber) ==
			GTK_RESPONSE_ACCEPT) {
		fns = gtk_file_chooser_get_filenames(
				GTK_FILE_CHOOSER (screen.win.gerber));
		/* Update the last folder */
		g_free (mainProject->path);
		mainProject->path = gtk_file_chooser_get_current_folder(
				(GtkFileChooser *)screen.win.gerber);
	}
	gtk_widget_destroy (screen.win.gerber);

	open_files (fns);
	g_slist_free_full (fns, g_free);
}