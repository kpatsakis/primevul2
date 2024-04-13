callbacks_clear_messages_button_clicked  (GtkButton *button, gpointer   user_data) {
	GtkTextBuffer *textbuffer;
	GtkTextIter start, end;

	screen.length_sum = 0;

	textbuffer = gtk_text_view_get_buffer((GtkTextView*)screen.win.messageTextView);
	gtk_text_buffer_get_start_iter(textbuffer, &start);
	gtk_text_buffer_get_end_iter(textbuffer, &end);
	gtk_text_buffer_delete (textbuffer, &start, &end);
}