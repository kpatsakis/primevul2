void av_fast_padded_mallocz(void *ptr, unsigned int *size, size_t min_size)
{
    uint8_t **p = ptr;
    if (min_size > SIZE_MAX - AV_INPUT_BUFFER_PADDING_SIZE) {
        av_freep(p);
        *size = 0;
        return;
    }
    if (!ff_fast_malloc(p, size, min_size + AV_INPUT_BUFFER_PADDING_SIZE, 1))
        memset(*p, 0, min_size + AV_INPUT_BUFFER_PADDING_SIZE);
}
