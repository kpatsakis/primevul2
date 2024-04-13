void HierarchicalBitmapRequester::PostImageHeight(ULONG lines)
{
  BitmapCtrl::PostImageHeight(lines);
#if ACCUSOFT_CODE
  assert(m_pulHeight);

  if (m_pLargestScale)
    m_pLargestScale->PostImageHeight(lines);
  
  for(UBYTE i = 0;i < m_ucCount;i++) {
    class Component *comp = m_pFrame->ComponentOf(i);
    UBYTE suby            = comp->SubYOf();
    m_pulHeight[i]        = (m_ulPixelHeight + suby - 1) / suby;
  }
#endif
}