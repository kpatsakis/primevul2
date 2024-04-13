callbacks_begin_print (GtkPrintOperation *operation, GtkPrintContext   *context,
		gpointer user_data) {
	gtk_print_operation_set_n_pages     (operation, 1);
}