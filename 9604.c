callbacks_sidepane_render_type_combo_box_changed (GtkComboBox *widget, gpointer user_data) {
	gerbv_render_types_t type = gtk_combo_box_get_active (widget);
	
	dprintf ("%s():  type = %d\n", __FUNCTION__, type);

	if (type < 0 || type == screenRenderInfo.renderType)
		return;

	screenRenderInfo.renderType = type;
	callbacks_render_type_changed ();
}