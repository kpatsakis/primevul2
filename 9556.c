callbacks_quit_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  gboolean layers_dirty = FALSE;
  gint idx;

  for (idx = 0; idx<=mainProject->last_loaded; idx++) {
    if (mainProject->file[idx] == NULL) break;
    layers_dirty = layers_dirty || mainProject->file[idx]->layer_dirty;
  }

  if (layers_dirty &&
      !interface_get_alert_dialog_response(
		_("Do you want to close all open layers and quit the program?"),
		_("Quitting the program will cause any unsaved changes "
		"to be lost."),
		FALSE, NULL, GTK_STOCK_QUIT, GTK_STOCK_CANCEL)) {
    return TRUE; // stop propagation of the delete_event.
	// this would destroy the gui but not return from the gtk event loop.
  }

  /* Save background color */
  if (screen.settings && !screen.background_is_from_project) {
    guint clr;
    GdkColor *bg = &mainProject->background;

    clr = bg->red/257<<16 | bg->green/257<<8 | bg->blue/257;
    g_settings_set_uint (screen.settings, "background-color", clr);
  }

  /* Save main window size and postion */
  if (screen.settings) {
    GtkWindow *win = GTK_WINDOW(screen.win.topLevelWindow);
    gint32 xy[2];
    GVariant *var;
    gboolean is_max;

    is_max = FALSE != (GDK_WINDOW_STATE_MAXIMIZED & gdk_window_get_state (
			    gtk_widget_get_window (GTK_WIDGET(win))));
    g_settings_set_boolean (screen.settings, "window-maximized", is_max);

    if (!is_max) {
      gtk_window_get_size (win, (gint *)xy, (gint *)(xy+1));
      var = g_variant_new_fixed_array (G_VARIANT_TYPE_INT32, xy, 2,
		      sizeof (xy[0]));
      g_settings_set_value (screen.settings, "window-size", var);

      gtk_window_get_position (win, (gint *)xy, (gint *)(xy+1));
      var = g_variant_new_fixed_array (G_VARIANT_TYPE_INT32, xy, 2,
		      sizeof (xy[0]));
      g_settings_set_value (screen.settings, "window-position", var);
    }
  }

  gerbv_unload_all_layers (mainProject);
  gtk_main_quit();

  return FALSE;
}