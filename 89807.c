size_t asocket::get_max_payload() const {
 size_t max_payload = MAX_PAYLOAD;
 if (transport) {
        max_payload = std::min(max_payload, transport->get_max_payload());
 }
 if (peer && peer->transport) {
        max_payload = std::min(max_payload, peer->transport->get_max_payload());
 }
 return max_payload;
}
