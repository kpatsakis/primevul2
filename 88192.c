void ConfigureHttp2Params(const base::CommandLine& command_line,
                          base::StringPiece http2_trial_group,
                          const VariationParameters& http2_trial_params,
                          net::HttpNetworkSession::Params* params) {
  if (GetVariationParam(http2_trial_params, "http2_enabled") == "false") {
    params->enable_http2 = false;
    return;
  }

  params->http2_settings = GetHttp2Settings(http2_trial_params);
  if (GetVariationParam(http2_trial_params, "http2_grease_settings") ==
      "true") {
    spdy::SpdySettingsId id = 0x0a0a + 0x1000 * base::RandGenerator(0xf + 1) +
                              0x0010 * base::RandGenerator(0xf + 1);
    uint32_t value = base::RandGenerator(
        static_cast<uint64_t>(std::numeric_limits<uint32_t>::max()) + 1);
    params->http2_settings.insert(std::make_pair(id, value));
  }

  if (GetVariationParam(http2_trial_params, "http2_grease_frame_type") ==
      "true") {
    const uint8_t type = 0x0b + 0x1f * base::RandGenerator(8);
    const uint8_t flags =
        base::RandGenerator(std::numeric_limits<uint8_t>::max() + 1);
    const size_t length = base::RandGenerator(7);
    const std::string payload =
        (length > 0) ? base::RandBytesAsString(length) : std::string();
    params->greased_http2_frame =
        base::Optional<net::SpdySessionPool::GreasedHttp2Frame>(
            {type, flags, payload});
  }

  params->enable_websocket_over_http2 =
      ConfigureWebsocketOverHttp2(command_line, http2_trial_params);
}
