void HierarchicalBitmapRequester::RequestUserDataForDecoding(class BitMapHook *bmh,RectAngle<LONG> &region,
                                                             const struct RectangleRequest *rr,bool alpha)
{
#if ACCUSOFT_CODE
  int i;

  ResetBitmaps();
  
  if (m_pLargestScale->FrameOf()->WidthOf()   != m_pFrame->WidthOf() ||
      (m_pLargestScale->FrameOf()->HeightOf() != m_pFrame->HeightOf() &&
       m_pLargestScale->FrameOf()->HeightOf() != 0 && m_pFrame->HeightOf() != 0)) {
    JPG_THROW(MALFORMED_STREAM,"HierarchicalBitmapRequester::ReconstructRegion",
              "hierarchical frame hierarchy is damaged, largest frame does not match the image");
  }
  
  if (m_ulPixelHeight == 0) {
    ULONG height = 0;
    if (m_pLargestScale->FrameOf()->HeightOf() != 0) {
      height = m_pLargestScale->FrameOf()->HeightOf();
    } else if (m_pFrame->HeightOf() != 0) {
      height = m_pFrame->HeightOf();
    }
    if (height) {
      PostImageHeight(height);
    }
  }
  
  m_ulMaxMCU = MAX_ULONG;
  
  for(i = rr->rr_usFirstComponent;i <= rr->rr_usLastComponent;i++) {
    RequestUserData(bmh,region,i,alpha);
    ULONG max = (BitmapOf(i).ibm_ulHeight >> 3) - 1;
    if (max < m_ulMaxMCU)
      m_ulMaxMCU = max;
  }
#else
  NOREF(bmh);
  NOREF(region);
  NOREF(rr);
  NOREF(alpha);
#endif
}