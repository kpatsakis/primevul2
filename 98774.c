static int read_next_ext_header(LHAFileHeader **header,
                                LHAInputStream *stream,
                                uint8_t **ext_header,
                                size_t *ext_header_len)
{

	*ext_header_len
	    = lha_decode_uint16(&RAW_DATA(header, RAW_DATA_LEN(header) - 2));


	if (*ext_header_len == 0) {
		*ext_header = NULL;
		return 1;
	}

	*ext_header = extend_raw_data(header, stream, *ext_header_len);

	return *ext_header != NULL;
}
