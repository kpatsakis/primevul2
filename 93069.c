static int snd_compr_allocate_buffer(struct snd_compr_stream *stream,
		struct snd_compr_params *params)
{
 	unsigned int buffer_size;
 	void *buffer;
 
	if (params->buffer.fragment_size == 0 ||
	    params->buffer.fragments > SIZE_MAX / params->buffer.fragment_size)
		return -EINVAL;

 	buffer_size = params->buffer.fragment_size * params->buffer.fragments;
 	if (stream->ops->copy) {
 		buffer = NULL;
		/* if copy is defined the driver will be required to copy
		 * the data from core
		 */
	} else {
		buffer = kmalloc(buffer_size, GFP_KERNEL);
		if (!buffer)
			return -ENOMEM;
	}
	stream->runtime->fragment_size = params->buffer.fragment_size;
	stream->runtime->fragments = params->buffer.fragments;
	stream->runtime->buffer = buffer;
	stream->runtime->buffer_size = buffer_size;
	return 0;
}