callbacks_save_layer_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  /* first figure out which layer in the layer side menu is selected */
  gint index=callbacks_get_selected_row_index();
  
  /* Now save that layer */
  if (index >= 0) {
    if (!gerbv_save_layer_from_index (mainProject, index, mainProject->file[index]->fullPathname)) {
      interface_show_alert_dialog(_("Gerbv cannot export this file type"), 
				  NULL,
				  FALSE,
				  NULL);
      mainProject->file[index]->layer_dirty = FALSE;
      callbacks_update_layer_tree();
      return;
    }
  }
  callbacks_update_layer_tree();
  return;
}