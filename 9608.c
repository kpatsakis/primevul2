callbacks_statusbar_unit_combo_box_changed (GtkComboBox *widget, gpointer user_data) {
	gerbv_gui_unit_t unit = gtk_combo_box_get_active (widget);
	int force_change = GPOINTER_TO_INT (user_data);

	if (!force_change && (unit < 0 || unit == screen.unit))
		return;

	callbacks_units_changed (unit);
}