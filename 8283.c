bool HierarchicalBitmapRequester::isNextMCULineReady(void) const
{
#if ACCUSOFT_CODE
  // MCUs can only be written if the smallest scale, which is written first,
  // is ready.
  return m_pSmallestScale->isNextMCULineReady();
#else
  return false;
#endif
}