analyze_window_size_store(GtkWidget *win, gpointer user_data)
{
	gint32 xy[2];
	GVariant *var;
	gboolean is_max;

	if (!screen.settings)
		return;

	is_max = FALSE != (GDK_WINDOW_STATE_MAXIMIZED &
			gdk_window_get_state (gtk_widget_get_window (win)));
	if (is_max)
		return;

	gtk_window_get_size (GTK_WINDOW (win), (gint *)xy, (gint *)(xy+1));
	var = g_variant_new_fixed_array (G_VARIANT_TYPE_INT32,
			xy, 2, sizeof (xy[0]));
	g_settings_set_value (screen.settings, "analyze-window-size", var);

	gtk_window_get_position (GTK_WINDOW (win),
			(gint *)xy, (gint *)(xy+1));
	var = g_variant_new_fixed_array (G_VARIANT_TYPE_INT32,
			xy, 2, sizeof (xy[0]));
	g_settings_set_value (screen.settings, "analyze-window-position", var);
}