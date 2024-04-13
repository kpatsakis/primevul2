bool HierarchicalBitmapRequester::isImageComplete(void) const
{ 
#if ACCUSOFT_CODE
  for(UBYTE i = 0;i < m_ucCount;i++) {
    if (m_pulReadyLines[i] < m_ulPixelHeight)
      return false;
  }
  return true;
#else
  return false;
#endif
}