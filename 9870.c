PreventStartCodeEmulation(const AP4_UI08* payload, AP4_Size payload_size, AP4_DataBuffer& output)
{
    output.Reserve(payload_size*2); // more than enough
    AP4_Size  output_size = 0;
    AP4_UI08* buffer = output.UseData();
    
	unsigned int zero_counter = 0;
	for (unsigned int i = 0; i < payload_size; i++) {
		if (zero_counter == 2) {
            if (payload[i] == 0 || payload[i] == 1 || payload[i] == 2 || payload[i] == 3) {
                buffer[output_size++] = 3;
                zero_counter = 0;
            }
		}

        buffer[output_size++] = payload[i];

		if (payload[i] == 0) {
			++zero_counter;
		} else {
			zero_counter = 0;
        }
	}

    output.SetDataSize(output_size);
}