callbacks_drawingarea_motion_notify_event (GtkWidget *widget, GdkEventMotion *event)
{
	int x, y;
	GdkModifierType state;

	if (event->is_hint)
		gdk_window_get_pointer (event->window, &x, &y, &state);
	else {
		x = event->x;
		y = event->y;
		state = event->state;
	}

	switch (screen.state) {
		case IN_MOVE: {
		    if (screen.last_x != 0 || screen.last_y != 0) {
			/* Move pixmap to get a snappier feel of movement */
			screen.off_x += x - screen.last_x;
			screen.off_y += y - screen.last_y;
		    }
    		    screenRenderInfo.lowerLeftX -= ((x - screen.last_x) / screenRenderInfo.scaleFactorX);
		    screenRenderInfo.lowerLeftY += ((y - screen.last_y) / screenRenderInfo.scaleFactorY);
		    callbacks_force_expose_event_for_screen ();
		    callbacks_update_scrollbar_positions ();
			screen.last_x = x;
			screen.last_y = y;
		    break;
		}
		case IN_ZOOM_OUTLINE: {
			if (screen.last_x || screen.last_y)
				render_draw_zoom_outline(screen.centered_outline_zoom);
			screen.last_x = x;
			screen.last_y = y;
			render_draw_zoom_outline(screen.centered_outline_zoom);
			break;
		}
		case IN_MEASURE: {
			/* clear the previous drawn line by drawing over it */
			render_toggle_measure_line();
			callbacks_screen2board(&(screen.measure_stop_x),
			    &(screen.measure_stop_y), x, y);
			/* screen.last_[xy] are updated to move the ruler pointers */
			screen.last_x = x;
			screen.last_y = y;
			/* draw the new line and write the new distance */
			render_draw_measure_distance();
			break;
		}
		case IN_SELECTION_DRAG: {
			if (screen.last_x || screen.last_y)
				render_draw_selection_box_outline();
			screen.last_x = x;
			screen.last_y = y;
			render_draw_selection_box_outline();
			break;
		}
		default:
			screen.last_x = x;
			screen.last_y = y;
			break;
	}
	callbacks_update_statusbar_coordinates (x, y);
	callbacks_update_ruler_pointers ();
	return TRUE;
} /* motion_notify_event */