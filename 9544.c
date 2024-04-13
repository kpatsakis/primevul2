callbacks_show_cross_on_drill_holes (GtkMenuItem *menuitem, gpointer user_data)
{
	screenRenderInfo.show_cross_on_drill_holes = GTK_CHECK_MENU_ITEM(menuitem)->active;
	render_refresh_rendered_image_on_screen();
}