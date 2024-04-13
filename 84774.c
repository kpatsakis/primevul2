void HTMLFormControlElement::disabledAttributeChanged() {
  setNeedsWillValidateCheck();
  pseudoStateChanged(CSSSelector::PseudoDisabled);
  pseudoStateChanged(CSSSelector::PseudoEnabled);
  if (layoutObject())
    LayoutTheme::theme().controlStateChanged(*layoutObject(),
                                             EnabledControlState);
  if (isDisabledFormControl() && adjustedFocusedElementInTreeScope() == this) {
    document().setNeedsFocusedElementCheck();
  }
}
