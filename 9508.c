callbacks_show_selection_on_invisible (GtkMenuItem *menuitem, gpointer user_data)
{
	mainProject->show_invisible_selection = GTK_CHECK_MENU_ITEM(menuitem)->active;
	render_refresh_rendered_image_on_screen();
}