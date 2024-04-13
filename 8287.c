HierarchicalBitmapRequester::HierarchicalBitmapRequester(class Frame *dimensions)
  : BitmapCtrl(dimensions)
#if ACCUSOFT_CODE
  , m_ppDownsampler(NULL), m_ppUpsampler(NULL), 
    m_ppTempIBM(NULL), m_pSmallestScale(NULL), m_pLargestScale(NULL), m_pTempAdapter(NULL),
    m_pulReadyLines(NULL), m_pulY(NULL), m_pulHeight(NULL),
    m_ppEncodingMCU(NULL), m_ppDecodingMCU(NULL),
    m_bSubsampling(false)
#endif
{
}