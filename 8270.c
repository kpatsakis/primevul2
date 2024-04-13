HierarchicalBitmapRequester::~HierarchicalBitmapRequester(void)
{
#if ACCUSOFT_CODE
  class LineAdapter *la;
  UBYTE i;
  
  if (m_ppEncodingMCU) {
    assert(m_pLargestScale);
    for(i = 0;i < (m_ucCount << 3);i++) {
      m_pLargestScale->DropLine(m_ppEncodingMCU[i],i >> 3);
    }
    m_pEnviron->FreeMem(m_ppEncodingMCU,sizeof(struct Line *) * m_ucCount * 8);
  }

  if (m_ppDecodingMCU) {
    assert(m_pLargestScale);
    for(i = 0;i < (m_ucCount << 3);i++) {
      m_pLargestScale->ReleaseLine(m_ppDecodingMCU[i],i >> 3);
    }
    m_pEnviron->FreeMem(m_ppDecodingMCU,sizeof(struct Line *) * m_ucCount * 8);
  }

  //
  // Dispose the tree of line adapters
  while((la = m_pLargestScale)) {
    m_pLargestScale = la->LowPassOf();
    delete la->HighPassOf();
    delete la;
  }
  delete m_pTempAdapter;
  
  if (m_ppDownsampler) {
    for(i = 0;i < m_ucCount;i++) {
      delete m_ppDownsampler[i];
    }
    m_pEnviron->FreeMem(m_ppDownsampler,m_ucCount * sizeof(class DownsamplerBase *));
  }

  if (m_ppUpsampler) {
    for(i = 0;i < m_ucCount;i++) {
      delete m_ppUpsampler[i];
    }
    m_pEnviron->FreeMem(m_ppUpsampler,m_ucCount * sizeof(class UpsamplerBase *));
  }

  if (m_ppTempIBM) {
    for(i = 0;i < m_ucCount;i++) {
      delete m_ppTempIBM[i];
    }
    m_pEnviron->FreeMem(m_ppTempIBM,m_ucCount * sizeof(struct ImageBitMap *));
  }

  if (m_pulReadyLines)
    m_pEnviron->FreeMem(m_pulReadyLines,m_ucCount * sizeof(ULONG));

  if (m_pulY)
    m_pEnviron->FreeMem(m_pulY,m_ucCount * sizeof(ULONG));

  if (m_pulHeight)
    m_pEnviron->FreeMem(m_pulHeight,m_ucCount * sizeof(ULONG));
#endif
}