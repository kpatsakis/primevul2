callbacks_print_render_page (GtkPrintOperation *operation,
           GtkPrintContext   *context,
           gint               page_nr,
           gpointer           user_data)
{
	GtkPrintSettings *pSettings = gtk_print_operation_get_print_settings (operation);
	gerbv_render_info_t renderInfo = {1.0, 1.0, 0, 0,
		GERBV_RENDER_TYPE_CAIRO_HIGH_QUALITY,
		(gint) gtk_print_context_get_width (context),
		(gint) gtk_print_context_get_height (context)};
	cairo_t *cr;
	
	/* have to assume x and y resolutions are the same for now, since we
	   don't support differing scales in the gerb_render_info_t struct yet */
	gdouble xres = gtk_print_context_get_dpi_x (context);
	gdouble yres = gtk_print_context_get_dpi_y (context);
	gdouble scalePercentage = gtk_print_settings_get_scale (pSettings);
	renderInfo.scaleFactorX = scalePercentage / 100 * xres;
	renderInfo.scaleFactorY = scalePercentage / 100 * yres;

	gerbv_render_translate_to_fit_display (mainProject, &renderInfo);
	cr = gtk_print_context_get_cairo_context (context);
	gerbv_render_all_layers_to_cairo_target_for_vector_output (mainProject, cr, &renderInfo);
}