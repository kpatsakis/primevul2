void ConfirmInfoBar::ButtonPressed(views::Button* sender,
                                   const ui::Event& event) {
  if (!owner())
    return;  // We're closing; don't call anything, it might access the owner.
  ConfirmInfoBarDelegate* delegate = GetDelegate();
  if (sender == ok_button_) {
    if (delegate->Accept())
      RemoveSelf();
  } else if (sender == cancel_button_) {
    if (delegate->Cancel())
      RemoveSelf();
  } else {
    InfoBarView::ButtonPressed(sender, event);
  }
}
