callbacks_show_toolbar_toggled (GtkMenuItem *menuitem, gpointer user_data)
{
	gtk_widget_set_visible (user_data, GTK_CHECK_MENU_ITEM(menuitem)->active);
}