void FeatureInfo::InitializeForTesting() {
  initialized_ = false;
  Initialize(CONTEXT_TYPE_OPENGLES2, false /* is_passthrough_cmd_decoder */,
             DisallowedFeatures());
}
