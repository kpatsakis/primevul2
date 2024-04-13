callbacks_move_layer_up_menu_activate (GtkMenuItem *menuitem, gpointer user_data) {
	callbacks_move_layer_up_button_clicked (NULL, NULL);
	gtk_widget_grab_focus (screen.win.layerTree);
}