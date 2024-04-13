callbacks_support_benchmark (gerbv_render_info_t *renderInfo) {
	int i;
	time_t start, now;
	GdkPixmap *renderedPixmap = gdk_pixmap_new (NULL, renderInfo->displayWidth,
								renderInfo->displayHeight, 24);
								
	// start by running the GDK (fast) rendering test
	i = 0;
	start = time(NULL);
	now = start;
	while( now - 30 < start) {
		i++;
		dprintf("Benchmark():  Starting redraw #%d\n", i);
		gerbv_render_to_pixmap_using_gdk (mainProject, renderedPixmap, renderInfo, NULL, NULL);
		now = time(NULL);
		dprintf("Elapsed time = %ld seconds\n", (long int) (now - start));
	}
	g_message(_("FAST (=GDK) mode benchmark: %d redraws "
				"in %ld seconds (%g redraws/second)"),
		      i, (long int) (now - start), (double) i / (double)(now - start));
	gdk_pixmap_unref(renderedPixmap);
	
	// run the cairo (normal) render mode
	i = 0;
	start = time(NULL);
	now = start;
	renderInfo->renderType = GERBV_RENDER_TYPE_CAIRO_NORMAL;
	while( now - 30 < start) {
		i++;
		dprintf("Benchmark():  Starting redraw #%d\n", i);
		cairo_surface_t *cSurface = cairo_image_surface_create  (CAIRO_FORMAT_ARGB32,
	                              renderInfo->displayWidth, renderInfo->displayHeight);
		cairo_t *cairoTarget = cairo_create (cSurface);
		gerbv_render_all_layers_to_cairo_target (mainProject, cairoTarget, renderInfo);
		cairo_destroy (cairoTarget);
		cairo_surface_destroy (cSurface);
		now = time(NULL);
		dprintf("Elapsed time = %ld seconds\n", (long int) (now - start));
	}
	g_message(_("NORMAL (=Cairo) mode benchmark: %d redraws "
				"in %ld seconds (%g redraws/second)"),
		      i, (long int) (now - start), (double) i / (double)(now - start));
}