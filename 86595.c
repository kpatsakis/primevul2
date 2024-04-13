void WebGLRenderingContextBase::TexImageBitmapByGPU(
    ImageBitmap* bitmap,
    GLenum target,
    GLuint target_texture,
    GLint xoffset,
    GLint yoffset,
    const IntRect& source_sub_rect) {
  bitmap->BitmapImage()->CopyToTexture(
      GetDrawingBuffer()->ContextProvider()->ContextGL(), target,
      target_texture, true /* unpack_premultiply_alpha */,
      false /* unpack_flip_y_ */, IntPoint(xoffset, yoffset), source_sub_rect);
}
