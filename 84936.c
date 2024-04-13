PDFEngineExports::RenderingSettings::RenderingSettings(int dpi_x,
                                                       int dpi_y,
                                                       const pp::Rect& bounds,
                                                       bool fit_to_bounds,
                                                       bool stretch_to_bounds,
                                                       bool keep_aspect_ratio,
                                                       bool center_in_bounds,
                                                       bool autorotate)
    : dpi_x(dpi_x),
      dpi_y(dpi_y),
      bounds(bounds),
      fit_to_bounds(fit_to_bounds),
      stretch_to_bounds(stretch_to_bounds),
      keep_aspect_ratio(keep_aspect_ratio),
      center_in_bounds(center_in_bounds),
      autorotate(autorotate) {}