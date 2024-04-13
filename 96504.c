static int mr_input_mapping(struct hid_device *hdev, struct hid_input *hi,
		struct hid_field *field, struct hid_usage *usage,
		unsigned long **bit, int *max)
{
	if ((usage->hid & HID_USAGE_PAGE) != HID_UP_CONSUMER)
		return 0;

	switch (usage->hid & HID_USAGE) {
	case 0x156: mr_map_key_clear(KEY_WORDPROCESSOR);	break;
	case 0x157: mr_map_key_clear(KEY_SPREADSHEET);		break;
	case 0x158: mr_map_key_clear(KEY_PRESENTATION);		break;
	case 0x15c: mr_map_key_clear(KEY_STOP);			break;
	default:
		return 0;
	}
	return 1;
}
