callbacks_save_project_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  if (mainProject->project)
    main_save_project_from_filename (mainProject, mainProject->project);
  else
    callbacks_generic_save_activate (menuitem, (gpointer) CALLBACKS_SAVE_PROJECT_AS);
  callbacks_update_layer_tree();
  return;
}