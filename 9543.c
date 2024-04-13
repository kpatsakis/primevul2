callbacks_viewmenu_rendertype_changed (GtkCheckMenuItem *widget, gpointer user_data) {
	gerbv_render_types_t type = GPOINTER_TO_INT(user_data);

	if (type == screenRenderInfo.renderType)
		return;

	dprintf ("%s():  type = %d\n", __FUNCTION__, type);

	screenRenderInfo.renderType = type;
	callbacks_render_type_changed ();
}