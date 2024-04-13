callbacks_scrollbar_button_pressed (GtkWidget *widget, GdkEventButton *event){
	//screen.last_x = ((GtkAdjustment *)screen.win.hAdjustment)->value;
	screen.state = SCROLLBAR;
	return FALSE;
}