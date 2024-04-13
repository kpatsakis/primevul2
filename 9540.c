callbacks_show_color_picker_dialog (gint index){
	screen.win.colorSelectionDialog = NULL;
	GtkColorSelectionDialog *cs= (GtkColorSelectionDialog *) gtk_color_selection_dialog_new (_("Select a color"));
	GtkColorSelection *colorsel = (GtkColorSelection *) cs->colorsel;
	
	screen.win.colorSelectionDialog = (GtkWidget *) cs;
	screen.win.colorSelectionIndex = index;
	if (index >= 0)
		gtk_color_selection_set_current_color (colorsel, &mainProject->file[index]->color);
	else
		gtk_color_selection_set_current_color (colorsel, &mainProject->background);
	if ((screenRenderInfo.renderType >= GERBV_RENDER_TYPE_CAIRO_NORMAL)&&(index >= 0)) {
		gtk_color_selection_set_has_opacity_control (colorsel, TRUE);
		gtk_color_selection_set_current_alpha (colorsel, mainProject->file[index]->alpha);
	}
	gtk_widget_show_all((GtkWidget *)cs);
	if (gtk_dialog_run ((GtkDialog*)cs) == GTK_RESPONSE_OK) {
		GtkColorSelection *colorsel = (GtkColorSelection *) cs->colorsel;
		gint rowIndex = screen.win.colorSelectionIndex;
		
		if (index >= 0) {
			gtk_color_selection_get_current_color (colorsel, &mainProject->file[rowIndex]->color);
			gdk_colormap_alloc_color(gdk_colormap_get_system(), &mainProject->file[rowIndex]->color, FALSE, TRUE);
		}
		else {
			gtk_color_selection_get_current_color (colorsel, &mainProject->background);
			gdk_colormap_alloc_color(gdk_colormap_get_system(), &mainProject->background, FALSE, TRUE);
		}
		if ((screenRenderInfo.renderType >= GERBV_RENDER_TYPE_CAIRO_NORMAL)&&(index >= 0)) {
			mainProject->file[rowIndex]->alpha = gtk_color_selection_get_current_alpha (colorsel);
		}
		
		callbacks_update_layer_tree ();
		render_refresh_rendered_image_on_screen();
	}
	gtk_widget_destroy ((GtkWidget *)cs);
	screen.win.colorSelectionDialog = NULL;
}