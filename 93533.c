void jas_seq2d_bindsub(jas_matrix_t *s, jas_matrix_t *s1, int xstart,
void jas_seq2d_bindsub(jas_matrix_t *s, jas_matrix_t *s1, jas_matind_t xstart,
  jas_matind_t ystart, jas_matind_t xend, jas_matind_t yend)
 {
 	jas_matrix_bindsub(s, s1, ystart - s1->ystart_, xstart - s1->xstart_,
 	  yend - s1->ystart_ - 1, xend - s1->xstart_ - 1);
 }