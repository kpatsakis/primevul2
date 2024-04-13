callbacks_switch_to_correct_cursor (void)
{
	GdkWindow *drawing_area_window = screen.drawing_area->window;
	GdkCursor *cursor;

	if (screen.state == IN_MOVE) {
		cursor = gdk_cursor_new(GDK_FLEUR);
		gdk_window_set_cursor(drawing_area_window, cursor);
		gdk_cursor_destroy(cursor);
		return;
	}
	else if (screen.state == IN_ZOOM_OUTLINE) {
		cursor = gdk_cursor_new(GDK_SIZING);
		gdk_window_set_cursor(drawing_area_window, cursor);
		gdk_cursor_destroy(cursor);
		return;
	}
	callbacks_switch_to_normal_tool_cursor (screen.tool);
}