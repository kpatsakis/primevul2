void HierarchicalBitmapRequester::CropEncodingRegion(RectAngle<LONG> &region,const struct RectangleRequest *)
{ 
#if ACCUSOFT_CODE
  int i;

  ClipToImage(region);

  // Find the region to request.
  for(i = 0;i < m_ucCount;i++) {
    if (m_pulReadyLines[i] < ULONG(region.ra_MinY))
      region.ra_MinY = m_pulReadyLines[i];
  }
#else
  NOREF(region);
#endif
}