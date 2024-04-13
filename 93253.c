static av_cold int xpm_decode_close(AVCodecContext *avctx)
{
     XPMDecContext *x = avctx->priv_data;
     av_freep(&x->pixels);
 
    av_freep(&x->buf);
    x->buf_size = 0;

     return 0;
 }
