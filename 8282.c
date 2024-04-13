void HierarchicalBitmapRequester::ResetToStartOfImage(void)
{
#if ACCUSOFT_CODE
  for(UBYTE i = 0;i < m_ucCount;i++) {
    m_pulY[i] = 0;
    m_pulReadyLines[i] = 0;
  }
  //
  assert(m_pLargestScale);
  // Now iterate through the tree.
  m_pLargestScale->ResetToStartOfImage();
#endif
}