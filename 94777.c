static void red_channel_client_reset_send_data(RedChannelClient *rcc)
{
    spice_marshaller_reset(rcc->send_data.marshaller);
    rcc->send_data.header.data = spice_marshaller_reserve_space(rcc->send_data.marshaller,
                                                                rcc->send_data.header.header_size);
    spice_marshaller_set_base(rcc->send_data.marshaller, rcc->send_data.header.header_size);
    rcc->send_data.header.set_msg_type(&rcc->send_data.header, 0);
    rcc->send_data.header.set_msg_size(&rcc->send_data.header, 0);

    /* Keeping the serial consecutive: reseting it if reset_send_data
     * has been called before, but no message has been sent since then.
     */
    if (rcc->send_data.last_sent_serial != rcc->send_data.serial) {
        spice_assert(rcc->send_data.serial - rcc->send_data.last_sent_serial == 1);
        /*  When the urgent marshaller is active, the serial was incremented by
         *  the call to reset_send_data that was made for the main marshaller.
         *  The urgent msg receives this serial, and the main msg serial is
         *  the following one. Thus, (rcc->send_data.serial - rcc->send_data.last_sent_serial)
         *  should be 1 in this case*/
        if (!red_channel_client_urgent_marshaller_is_active(rcc)) {
            rcc->send_data.serial = rcc->send_data.last_sent_serial;
        }
    }
    rcc->send_data.serial++;

    if (!rcc->is_mini_header) {
        spice_assert(rcc->send_data.marshaller != rcc->send_data.urgent.marshaller);
        rcc->send_data.header.set_msg_sub_list(&rcc->send_data.header, 0);
        rcc->send_data.header.set_msg_serial(&rcc->send_data.header, rcc->send_data.serial);
    }
}
