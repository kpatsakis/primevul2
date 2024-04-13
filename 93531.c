int jas_matrix_resize(jas_matrix_t *matrix, int numrows, int numcols)
int jas_matrix_resize(jas_matrix_t *matrix, jas_matind_t numrows,
  jas_matind_t numcols)
 {
	jas_matind_t size;
	jas_matind_t i;
 
 	size = numrows * numcols;
 	if (size > matrix->datasize_ || numrows > matrix->maxrows_) {
		return -1;
	}

	matrix->numrows_ = numrows;
	matrix->numcols_ = numcols;

	for (i = 0; i < numrows; ++i) {
		matrix->rows_[i] = &matrix->data_[numcols * i];
	}

	return 0;
}
