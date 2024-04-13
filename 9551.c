callbacks_drawingarea_configure_event (GtkWidget *widget, GdkEventConfigure *event)
{
	GdkDrawable *drawable = widget->window;
	
	gdk_drawable_get_size (drawable, &screenRenderInfo.displayWidth, &screenRenderInfo.displayHeight);

	/* set this up if cairo is compiled, since we may need to switch over to
	   using the surface at any time */
	int x_off=0, y_off=0;
	
	if (GDK_IS_WINDOW(widget->window)) {
	      /* query the window's backbuffer if it has one */
		GdkWindow *window = GDK_WINDOW(widget->window);
	      gdk_window_get_internal_paint_info (window, &drawable, &x_off, &y_off);
	}
	if (screen.windowSurface)
		cairo_surface_destroy ((cairo_surface_t *)
			screen.windowSurface);

#if defined(WIN32) || defined(QUARTZ)
	cairo_t *cairoTarget = gdk_cairo_create (GDK_WINDOW(widget->window));
	
	screen.windowSurface = cairo_get_target (cairoTarget);
	/* increase surface reference by one so it isn't freed when the cairo_t
	   is destroyed next */
	screen.windowSurface = cairo_surface_reference (screen.windowSurface);
	cairo_destroy (cairoTarget);
#else
	GdkVisual *visual = gdk_drawable_get_visual (drawable);
	screen.windowSurface = (gpointer) cairo_xlib_surface_create (GDK_DRAWABLE_XDISPLAY (drawable),
                                          GDK_DRAWABLE_XID (drawable),
                                          GDK_VISUAL_XVISUAL (visual),
                                          screenRenderInfo.displayWidth,
                                          screenRenderInfo.displayHeight);
#endif
	/* if this is the first time, go ahead and call autoscale even if we don't
	   have a model loaded */
	if ((screenRenderInfo.scaleFactorX < 0.001)||(screenRenderInfo.scaleFactorY < 0.001)) {
		gerbv_render_zoom_to_fit_display (mainProject, &screenRenderInfo);
	}
	render_refresh_rendered_image_on_screen();
	return TRUE;
}