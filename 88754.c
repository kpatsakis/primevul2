DOMHighResTimeStamp PerformanceNavigationTiming::loadEventStart() const {
  DocumentLoadTiming* timing = GetDocumentLoadTiming();
  if (!timing)
    return 0.0;
  return Performance::MonotonicTimeToDOMHighResTimeStamp(
      TimeOrigin(), timing->LoadEventStart(), false /* allow_negative_value */);
}
