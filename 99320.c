void lcdInit_ArrayBuffer(JsGraphics *gfx) {
  JsVar *buf = jswrap_arraybuffer_constructor(graphicsGetMemoryRequired(gfx));
  jsvUnLock2(jsvAddNamedChild(gfx->graphicsVar, buf, "buffer"), buf);
}
