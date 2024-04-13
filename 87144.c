float PrintRenderFrameHelper::RenderPageContent(blink::WebLocalFrame* frame,
                                                int page_number,
                                                const gfx::Rect& canvas_area,
                                                const gfx::Rect& content_area,
                                                double scale_factor,
                                                blink::WebCanvas* canvas) {
  cc::PaintCanvasAutoRestore auto_restore(canvas, true);
  canvas->translate((content_area.x() - canvas_area.x()) / scale_factor,
                    (content_area.y() - canvas_area.y()) / scale_factor);
  return frame->PrintPage(page_number, canvas);
}
