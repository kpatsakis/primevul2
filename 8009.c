static inline int route4_fastmap_hash(u32 id, int iif)
{
	return id & 0xF;
}