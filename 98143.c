static void vp78_update_pred16x16_pred8x8_mvc_probabilities(VP8Context *s,
                                                            int mvc_size)
{
    VP56RangeCoder *c = &s->c;
    int i, j;

    if (vp8_rac_get(c))
        for (i = 0; i < 4; i++)
            s->prob->pred16x16[i] = vp8_rac_get_uint(c, 8);
    if (vp8_rac_get(c))
        for (i = 0; i < 3; i++)
            s->prob->pred8x8c[i]  = vp8_rac_get_uint(c, 8);

    for (i = 0; i < 2; i++)
        for (j = 0; j < mvc_size; j++)
            if (vp56_rac_get_prob_branchy(c, vp8_mv_update_prob[i][j]))
                s->prob->mvc[i][j] = vp8_rac_get_nn(c);
}
