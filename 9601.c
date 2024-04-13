callbacks_switch_to_normal_tool_cursor (gint toolNumber)
{
	GdkWindow *drawing_area_window = screen.drawing_area->window;
	GdkCursor *cursor;

	switch (toolNumber) {
	case POINTER:
		gdk_window_set_cursor(drawing_area_window, GERBV_DEF_CURSOR);
		break;
	case PAN:
		cursor = gdk_cursor_new(GDK_FLEUR);
		gdk_window_set_cursor(drawing_area_window, cursor);
		gdk_cursor_destroy(cursor);
		break;
	case ZOOM:
		cursor = gdk_cursor_new(GDK_SIZING);
		gdk_window_set_cursor(drawing_area_window, cursor);
		gdk_cursor_destroy(cursor);
		break;
	case MEASURE:
		cursor = gdk_cursor_new(GDK_CROSSHAIR);
		gdk_window_set_cursor(drawing_area_window, cursor);
		gdk_cursor_destroy(cursor);
		break;
	default:
		break;
	}
}