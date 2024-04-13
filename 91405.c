void sdp_disc_connected(tCONN_CB* p_ccb) {
 if (p_ccb->is_attr_search) {
    p_ccb->disc_state = SDP_DISC_WAIT_SEARCH_ATTR;

    process_service_search_attr_rsp(p_ccb, NULL, NULL);
 } else {
 /* First step is to get a list of the handles from the server. */
 /* We are not searching for a specific attribute, so we will   */
 /* first search for the service, then get all attributes of it */

    p_ccb->num_handles = 0;
    sdp_snd_service_search_req(p_ccb, 0, NULL);
 }
}
