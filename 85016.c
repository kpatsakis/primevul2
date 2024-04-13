void OmniboxViewViews::OnTabChanged(const content::WebContents* web_contents) {
  const OmniboxState* state = static_cast<OmniboxState*>(
      web_contents->GetUserData(&OmniboxState::kKey));
  model()->RestoreState(state ? &state->model_state : nullptr);
  if (state) {
    SelectRange(state->selection);
    saved_selection_for_focus_change_ = state->saved_selection_for_focus_change;
  }

  ClearEditHistory();
}