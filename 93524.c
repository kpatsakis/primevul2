 void jas_matrix_asr(jas_matrix_t *matrix, int n)
 {
	jas_matind_t i;
	jas_matind_t j;
 	jas_seqent_t *rowstart;
	jas_matind_t rowstep;
 	jas_seqent_t *data;
 
 	assert(n >= 0);
	if (jas_matrix_numrows(matrix) > 0 && jas_matrix_numcols(matrix) > 0) {
		assert(matrix->rows_);
		rowstep = jas_matrix_rowstep(matrix);
		for (i = matrix->numrows_, rowstart = matrix->rows_[0]; i > 0; --i,
		  rowstart += rowstep) {
			for (j = matrix->numcols_, data = rowstart; j > 0; --j,
			  ++data) {
				*data = jas_seqent_asr(*data, n);
			}
		}
	}
}