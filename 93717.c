static int er_supported(ERContext *s)
 {
     if(s->avctx->hwaccel && s->avctx->hwaccel->decode_slice           ||
        !s->cur_pic.f                                                  ||
       s->cur_pic.field_picture
     )
         return 0;
     return 1;
}
