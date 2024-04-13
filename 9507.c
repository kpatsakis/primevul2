callbacks_viewmenu_units_changed (GtkCheckMenuItem *widget, gpointer user_data) {
	gerbv_gui_unit_t unit = GPOINTER_TO_INT(user_data);

	if (unit < 0 || unit == screen.unit)
		return;

	dprintf ("%s():  unit = %d, screen.unit = %d\n", __FUNCTION__, unit, screen.unit);

	callbacks_units_changed (unit);
}