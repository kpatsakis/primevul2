callbacks_window_scroll_event(GtkWidget *widget, GdkEventScroll *event)
{
	switch (event->direction) {
		case GDK_SCROLL_UP:
			render_zoom_display (ZOOM_IN_CMOUSE, 0, event->x, event->y);
			break;
		case GDK_SCROLL_DOWN:
			render_zoom_display (ZOOM_OUT_CMOUSE, 0, event->x, event->y);
			break;
		case GDK_SCROLL_LEFT: 
			/* Ignore */
		case GDK_SCROLL_RIGHT:
			/* Ignore */
		default:
			return TRUE;
	}
	return TRUE;
} /* scroll_event */