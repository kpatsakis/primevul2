bool OmniboxViewViews::SelectedSuggestionHasTabMatch() const {
  return model()->popup_model() &&  // Can be null in tests.
         model()->popup_model()->SelectedLineHasButton();
}
