GPMF_ERR GPMF_SeekToSamples(GPMF_stream *ms)
{
	GPMF_stream prevstate;

	if (ms)
	{

		if (ms->pos+1 < ms->buffer_size_longs)
		{
			uint32_t type = GPMF_SAMPLE_TYPE(ms->buffer[ms->pos + 1]);

			memcpy(&prevstate, ms, sizeof(GPMF_stream));

			if (type == GPMF_TYPE_NEST)
				GPMF_Next(ms, GPMF_RECURSE_LEVELS); // open STRM and recurse in

			while (0 == GPMF_Next(ms, GPMF_CURRENT_LEVEL))
			{
				uint32_t size = (GPMF_DATA_SIZE(ms->buffer[ms->pos + 1]) >> 2);
				if (GPMF_OK != IsValidSize(ms, size))
				{
					memcpy(ms, &prevstate, sizeof(GPMF_stream));
					return GPMF_ERROR_BAD_STRUCTURE;
				}

				type = GPMF_SAMPLE_TYPE(ms->buffer[ms->pos + 1]);


				if (type == GPMF_TYPE_NEST)  // Nest with-in nest
				{
					return GPMF_OK; //found match
				}

				if (size + 2 == ms->nest_size[ms->nest_level])
				{
					uint32_t key = GPMF_Key(ms);

					if (GPMF_ERROR_RESERVED == GPMF_Reserved(key))
						return GPMF_ERROR_FIND;
					
					return GPMF_OK; //found match
				}

				if (ms->buffer[ms->pos] == ms->buffer[ms->pos + size + 2]) // Matching tags
				{
					return GPMF_OK; //found match
				}
			}

			memcpy(ms, &prevstate, sizeof(GPMF_stream));
			return GPMF_ERROR_FIND;
		}
	}
	return GPMF_ERROR_FIND;
}
