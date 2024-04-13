callbacks_scrollbar_button_released (GtkWidget *widget, GdkEventButton *event){
	screen.off_x = 0;
	screen.off_y = 0;
	screen.state = NORMAL;
	render_refresh_rendered_image_on_screen();
	return FALSE;
}