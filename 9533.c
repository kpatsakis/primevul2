analyze_window_size_restore(GtkWidget *win)
{
	GVariant *var;
	const gint32 *xy;
	gsize num;

	if (!screen.settings)
		return;

	var = g_settings_get_value (screen.settings, "analyze-window-size");
	xy = g_variant_get_fixed_array (var, &num, sizeof (*xy));
	if (num == 2)
		gtk_window_set_default_size (GTK_WINDOW (win), xy[0], xy[1]);
	g_variant_unref (var);

	var = g_settings_get_value (screen.settings, "analyze-window-position");
	xy = g_variant_get_fixed_array (var, &num, sizeof (*xy));
	if (num == 2)
		gtk_window_move (GTK_WINDOW (win), xy[0], xy[1]);
	g_variant_unref (var);
}