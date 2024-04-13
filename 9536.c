callbacks_layer_tree_visibility_toggled (gint index)
{
	mainProject->file[index]->isVisible =
		!mainProject->file[index]->isVisible;

	callbacks_update_layer_tree ();
	if (screenRenderInfo.renderType <= GERBV_RENDER_TYPE_GDK_XOR) {
		render_refresh_rendered_image_on_screen ();
	} else {
		render_recreate_composite_surface (screen.drawing_area);
		callbacks_force_expose_event_for_screen ();
	}
}