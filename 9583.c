callbacks_change_tool (GtkButton *button, gpointer   user_data) {
	gint toolNumber = GPOINTER_TO_INT (user_data);
	
	/* make sure se don't get caught in endless recursion here */
	if (screen.win.updatingTools)
		return;
	screen.win.updatingTools = TRUE;
	gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON (screen.win.toolButtonPointer), FALSE);
	gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON (screen.win.toolButtonPan), FALSE);
	gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON (screen.win.toolButtonZoom), FALSE);
	gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON (screen.win.toolButtonMeasure), FALSE);
	switch (toolNumber) {
		case POINTER:
			gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON (screen.win.toolButtonPointer), TRUE);
			screen.tool = POINTER;
			screen.state = NORMAL;
			utf8_strncpy(screen.statusbar.diststr,
				_("Click to select objects in the active layer. "
					"Middle click and drag to pan."),
				MAX_DISTLEN);
			break;
		case PAN:
			gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON (screen.win.toolButtonPan), TRUE);
			screen.tool = PAN;
			screen.state = NORMAL;
			utf8_strncpy(screen.statusbar.diststr,
				_("Click and drag to pan. Right click and drag to zoom."),
				MAX_DISTLEN);
			break;
		case ZOOM:
			gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON (screen.win.toolButtonZoom), TRUE);
			screen.tool = ZOOM;
			screen.state = NORMAL;
			utf8_strncpy(screen.statusbar.diststr,
				_("Click and drag to zoom in. Shift+click to zoom out."),
				MAX_DISTLEN);
			break;

		case MEASURE:
			gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON (screen.win.toolButtonMeasure), TRUE);
			screen.tool = MEASURE;
			screen.state = NORMAL;
			utf8_strncpy(screen.statusbar.diststr,
				_("Click and drag to measure a distance or select two apertures."),
				MAX_DISTLEN);

			/* To not show previous measure drag-line */
			screen.measure_start_x = 0;
			screen.measure_start_y = 0;
			screen.measure_stop_x =  0;
			screen.measure_stop_y =  0;

			/* If two items are selected, measure they distance */
			if (selection_length (&screen.selectionInfo) == 2) {
				gerbv_selection_item_t item[2];
				gerbv_net_t *net[2];

				item[0] = selection_get_item_by_index(
						&screen.selectionInfo, 0);
				item[1] = selection_get_item_by_index(
						&screen.selectionInfo, 1);
				net[0] = item[0].net;
				net[1] = item[1].net;

				if ((net[0]->aperture_state ==
						net[1]->aperture_state)
				 && (net[0]->aperture_state ==
						GERBV_APERTURE_STATE_FLASH)) {
					screen.measure_start_x = net[0]->stop_x;
					screen.measure_start_y = net[0]->stop_y;
					gerbv_transform_coord_for_image(
							&screen.measure_start_x,
							&screen.measure_start_y,
							item[0].image,
							mainProject);

					screen.measure_stop_x = net[1]->stop_x;
					screen.measure_stop_y = net[1]->stop_y;
					gerbv_transform_coord_for_image(
							&screen.measure_stop_x,
							&screen.measure_stop_y,
							item[1].image,
							mainProject);

					render_draw_measure_distance();
				}
			}
			break;
		default:
			break;
	}

	callbacks_switch_to_normal_tool_cursor (toolNumber);
	callbacks_update_statusbar();
	screen.win.updatingTools = FALSE;
	callbacks_force_expose_event_for_screen();
}