callbacks_live_edit (GtkWidget *button, gpointer user_data){
	GtkDialog *toplevel = GTK_DIALOG(gtk_widget_get_toplevel (button));
	gtk_dialog_response(toplevel, GTK_RESPONSE_APPLY);
}