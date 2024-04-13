AuthenticatorTransportSelectorSheetModel::GetStepIllustration(
    ImageColorScheme color_scheme) const {
  return color_scheme == ImageColorScheme::kDark ? kWebauthnWelcomeDarkIcon
                                                 : kWebauthnWelcomeIcon;
}
