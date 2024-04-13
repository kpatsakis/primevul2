callbacks_units_changed (gerbv_gui_unit_t unit)
{
	static gboolean isChanging = FALSE;

	if (isChanging)
		return;

	isChanging = TRUE;
	screen.unit = unit;

	if (unit == GERBV_MILS || unit == GERBV_MMS || unit == GERBV_INS) {
		gtk_combo_box_set_active (
				GTK_COMBO_BOX (screen.win.statusUnitComboBox),
				unit);
		gtk_check_menu_item_set_active (
				screen.win.menu_view_unit_group[unit], TRUE);
	}

	callbacks_update_ruler_scales ();
	callbacks_update_statusbar_coordinates (screen.last_x, screen.last_y);
	
	if (screen.tool == MEASURE)
		callbacks_update_statusbar_measured_distance (
				screen.measure_last_x,
				screen.measure_last_y);

	isChanging = FALSE;
}