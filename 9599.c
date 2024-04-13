callbacks_fullscreen_toggled (GtkMenuItem *menuitem, gpointer user_data)
{
	//struct GtkWindow *win = (struct GtkWindow *)(screen.win.topLevelWindow);
	GdkWindowState state = gdk_window_get_state (gtk_widget_get_window(screen.win.topLevelWindow));
	if(state & GDK_WINDOW_STATE_FULLSCREEN)
		gtk_window_unfullscreen (GTK_WINDOW(screen.win.topLevelWindow));
	else
		gtk_window_fullscreen (GTK_WINDOW(screen.win.topLevelWindow));
}