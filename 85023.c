void OmniboxViewViews::SaveStateToTab(content::WebContents* tab) {
  DCHECK(tab);

  if (IsIMEComposing()) {
    ConfirmCompositionText();
    GetInputMethod()->CancelComposition(this);
  }

  OmniboxEditModel::State state = model()->GetStateForTabSwitch();
  tab->SetUserData(OmniboxState::kKey, std::make_unique<OmniboxState>(
                                           state, GetSelectedRange(),
                                           saved_selection_for_focus_change_));
}
