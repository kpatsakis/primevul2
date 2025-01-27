static void ims_pcu_process_data(struct ims_pcu *pcu, struct urb *urb)
{
	int i;

	for (i = 0; i < urb->actual_length; i++) {
		u8 data = pcu->urb_in_buf[i];

		/* Skip everything until we get Start Xmit */
		if (!pcu->have_stx && data != IMS_PCU_PROTOCOL_STX)
			continue;

		if (pcu->have_dle) {
			pcu->have_dle = false;
			pcu->read_buf[pcu->read_pos++] = data;
			pcu->check_sum += data;
			continue;
		}

		switch (data) {
		case IMS_PCU_PROTOCOL_STX:
			if (pcu->have_stx)
				dev_warn(pcu->dev,
					 "Unexpected STX at byte %d, discarding old data\n",
					 pcu->read_pos);
			pcu->have_stx = true;
			pcu->have_dle = false;
			pcu->read_pos = 0;
			pcu->check_sum = 0;
			break;

		case IMS_PCU_PROTOCOL_DLE:
			pcu->have_dle = true;
			break;

		case IMS_PCU_PROTOCOL_ETX:
			if (pcu->read_pos < IMS_PCU_MIN_PACKET_LEN) {
				dev_warn(pcu->dev,
					 "Short packet received (%d bytes), ignoring\n",
					 pcu->read_pos);
			} else if (pcu->check_sum != 0) {
				dev_warn(pcu->dev,
					 "Invalid checksum in packet (%d bytes), ignoring\n",
					 pcu->read_pos);
			} else {
				ims_pcu_handle_response(pcu);
			}

			pcu->have_stx = false;
			pcu->have_dle = false;
			pcu->read_pos = 0;
			break;

		default:
			pcu->read_buf[pcu->read_pos++] = data;
			pcu->check_sum += data;
			break;
		}
	}
}
