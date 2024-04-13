njs_reserved(const uint32_t *reserve, uint32_t byte)
{
    return ((reserve[byte >> 5] & ((uint32_t) 1 << (byte & 0x1f))) != 0);
}