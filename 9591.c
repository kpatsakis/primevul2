callbacks_render_type_changed () {
	static gboolean isChanging = FALSE;
	if (isChanging)
		return;

	isChanging = TRUE;
	gerbv_render_types_t type = screenRenderInfo.renderType;
	GtkCheckMenuItem *check_item = screen.win.menu_view_render_group[type];
	dprintf ("%s():  type = %d, check_item = %p\n", __FUNCTION__, type, check_item);
	gtk_check_menu_item_set_active (check_item, TRUE);
	gtk_combo_box_set_active (screen.win.sidepaneRenderComboBox, type);

	render_refresh_rendered_image_on_screen();
	isChanging = FALSE;
}