callbacks_update_statusbar(void)
{
	if ((screen.statusbar.coordstr != NULL)&&(GTK_IS_LABEL(screen.win.statusMessageLeft))) {
		gtk_label_set_text(GTK_LABEL(screen.win.statusMessageLeft), screen.statusbar.coordstr);
	}
	if ((screen.statusbar.diststr != NULL)&&(GTK_IS_LABEL(screen.win.statusMessageRight))) {
		gtk_label_set_markup(GTK_LABEL(screen.win.statusMessageRight), screen.statusbar.diststr);
	}
}