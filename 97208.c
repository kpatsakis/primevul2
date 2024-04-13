static inline bool serpent_fpu_begin(bool fpu_enabled, unsigned int nbytes)
{
	/* since reusing AVX functions, starts using FPU at 8 parallel blocks */
	return glue_fpu_begin(SERPENT_BLOCK_SIZE, 8, NULL, fpu_enabled, nbytes);
}
