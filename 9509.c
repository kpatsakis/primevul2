callbacks_drawingarea_button_press_event (GtkWidget *widget, GdkEventButton *event)
{
	GdkWindow *drawing_area_window = screen.drawing_area->window;
	GdkCursor *cursor;
	
	switch (event->button) {
		case 1 :
			if (screen.tool == POINTER) {
				/* select */
				/* selection will only work with cairo, so do nothing if it's
				   not compiled */
				screen.state = IN_SELECTION_DRAG;
				screen.start_x = event->x;
				screen.start_y = event->y;
			}
			else if (screen.tool == PAN) {
				/* Plain panning */
				screen.state = IN_MOVE;
				screen.last_x = event->x;
				screen.last_y = event->y;
			}
			else if (screen.tool == ZOOM) {
				screen.state = IN_ZOOM_OUTLINE;
				/* Zoom outline mode initiated */
				screen.start_x = event->x;
				screen.start_y = event->y;
				screen.centered_outline_zoom = event->state;
			}
			else if (screen.tool == MEASURE) {
				screen.state = IN_MEASURE;
				callbacks_screen2board(&(screen.measure_start_x), &(screen.measure_start_y),
								event->x, event->y);
				screen.measure_stop_x = screen.measure_start_x;
				screen.measure_stop_y = screen.measure_start_y;
				/* force an expose event to clear any previous measure lines */
				callbacks_force_expose_event_for_screen ();
			}
			break;
		case 2 :
			screen.state = IN_MOVE;
			screen.last_x = event->x;
			screen.last_y = event->y;
			cursor = gdk_cursor_new(GDK_FLEUR);
			gdk_window_set_cursor(drawing_area_window, cursor);
			gdk_cursor_destroy(cursor);
			break;
		case 3 :
			if (screen.tool == POINTER) {
				/* if no items are selected, try and find the item the user
				   is pointing at */
				if (selection_length (&screen.selectionInfo) == 0) {
					gint index=callbacks_get_selected_row_index();
					if ((index >= 0) && 
					    (index <= mainProject->last_loaded) &&
					    (mainProject->file[index]->isVisible)) {
					  render_fill_selection_buffer_from_mouse_click(
							  event->x, event->y,
							  index, SELECTION_REPLACE);
					} else {
					    selection_clear (&screen.selectionInfo);
					    update_selected_object_message (FALSE);
					    render_refresh_rendered_image_on_screen ();
					}
				}

				/* only show the popup if we actually have something selected now */
				if (selection_length (&screen.selectionInfo) != 0) {
					update_selected_object_message (TRUE);
					gtk_menu_popup(GTK_MENU(screen.win.drawWindowPopupMenu), NULL, NULL, NULL, NULL, 
							event->button, event->time);
				}

			} else {
				/* Zoom outline mode initiated */
				screen.state = IN_ZOOM_OUTLINE;
				screen.start_x = event->x;
				screen.start_y = event->y;
				screen.centered_outline_zoom = event->state & GDK_SHIFT_MASK;
				cursor = gdk_cursor_new(GDK_SIZING);
				gdk_window_set_cursor(drawing_area_window, cursor);
				gdk_cursor_destroy(cursor);
			}
			break;
		case 4 : /* Scroll wheel */
			render_zoom_display (ZOOM_IN_CMOUSE, 0, event->x, event->y);
			break;
		case 5 :  /* Scroll wheel */
			render_zoom_display (ZOOM_OUT_CMOUSE, 0, event->x, event->y);
			break;
		default:
			break;
	}
	callbacks_switch_to_correct_cursor ();
	return TRUE;
}