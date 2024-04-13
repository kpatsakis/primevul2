ACodec::BaseState::PortMode ACodec::ExecutingState::getPortMode(
        OMX_U32 /* portIndex */) {
 return RESUBMIT_BUFFERS;
}
