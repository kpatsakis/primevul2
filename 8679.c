static bool needs_escape(unsigned char c)
{
	return c == BYTE_ESC || c == BYTE_FRAME;
}