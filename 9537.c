callbacks_fit_to_window_activate              (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	gerbv_render_zoom_to_fit_display (mainProject, &screenRenderInfo);
	render_refresh_rendered_image_on_screen();
}