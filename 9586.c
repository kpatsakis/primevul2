callbacks_drawingarea_button_release_event (GtkWidget *widget, GdkEventButton *event)
{
	gint index;

	if (event->type != GDK_BUTTON_RELEASE)
		return TRUE;

	switch (screen.state) {
	case IN_MOVE:
		screen.off_x = 0;
		screen.off_y = 0;
		render_refresh_rendered_image_on_screen ();
		callbacks_switch_to_normal_tool_cursor (screen.tool);
		break;

	case IN_ZOOM_OUTLINE:
		if ((event->state & GDK_SHIFT_MASK) != 0) {
			render_zoom_display (ZOOM_OUT_CMOUSE, 0,
					event->x, event->y);
		}
		/* if the user just clicks without dragging, then simply
		   zoom in a preset amount */
		else if ((abs(screen.start_x - event->x) < 4) &&
				(abs(screen.start_y - event->y) < 4)) {
			render_zoom_display (ZOOM_IN_CMOUSE, 0,
					event->x, event->y);
		} else {
			render_calculate_zoom_from_outline (widget, event);
		}
		callbacks_switch_to_normal_tool_cursor (screen.tool);
		break;

	case IN_SELECTION_DRAG:
		/* selection will only work with cairo, so do nothing if it's
		   not compiled */
		index = callbacks_get_selected_row_index ();

		if ((index >= 0) && mainProject->file[index]->isVisible) {
			enum selection_action sel_action = SELECTION_REPLACE;
			
			if (event->state & GDK_SHIFT_MASK)
				sel_action = SELECTION_ADD;
			else if (event->state & GDK_CONTROL_MASK)
				sel_action = SELECTION_TOGGLE;

			/* determine if this was just a click or a box drag */
			if ((fabs((double)(screen.last_x - screen.start_x)) < 5)
			 && (fabs((double)(screen.last_y - screen.start_y)) < 5)) {
				render_fill_selection_buffer_from_mouse_click (
						event->x, event->y, index,
						sel_action);
			} else {
				render_fill_selection_buffer_from_mouse_drag (
						event->x, event->y,
						screen.start_x, screen.start_y,
						index, sel_action);
			}

			/* Check if anything was selected */
			update_selected_object_message (TRUE);

			check_align_files_possibility (&screen.selectionInfo);
		} else {
			render_refresh_rendered_image_on_screen ();
		}
		break;
	default:
		break;
	}

	screen.state = NORMAL;

	return TRUE;
} /* button_release_event */