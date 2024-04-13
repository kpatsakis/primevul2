void FeatureInfo::InitializeForTesting(
    const DisallowedFeatures& disallowed_features) {
  initialized_ = false;
  Initialize(CONTEXT_TYPE_OPENGLES2, false /* is_passthrough_cmd_decoder */,
             disallowed_features);
}
