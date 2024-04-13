  void ParseCommandLineAndFieldTrials(const base::CommandLine& command_line) {
    network_session_configurator::ParseCommandLineAndFieldTrials(
        command_line,
        /*is_quic_force_disabled=*/false, quic_user_agent_id_, &params_);
  }
