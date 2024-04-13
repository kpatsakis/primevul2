void HierarchicalBitmapRequester::AddImageScale(class Frame *frame,bool expandh,bool expandv)
{
#if ACCUSOFT_CODE
  if (m_pLargestScale == NULL) {
    assert(m_pSmallestScale == NULL);
    assert(expandh == false && expandv == false);
    // Actually, this is the smallest scale... as it is the first we build.
    m_pLargestScale  = frame->BuildLineAdapter();
    m_pSmallestScale = m_pLargestScale;
    frame->SetImageBuffer(m_pLargestScale);
  } else {
    class LineMerger *merger;
    // Two things need to be build: The adapter to the new band, and the merger
    // that merges this band with the output and scales the result
    // apropriately.
    assert(m_pTempAdapter == NULL);
    // This object will pull out lines from the new high-pass...
    m_pTempAdapter   = frame->BuildLineAdapter();
    // ...and this guy will merge them with what we currently have.
    merger           = new(m_pEnviron) class LineMerger(frame,m_pLargestScale,m_pTempAdapter,
                                                        expandh,expandv);
    //
    // And this becomes the next largest scale.
    m_pLargestScale  = merger;
    // and controls now the life-time of its children.
    frame->SetImageBuffer(m_pTempAdapter);
    m_pTempAdapter   = NULL; 
  }
#else
  NOREF(frame);
  NOREF(expandh);
  NOREF(expandv);
#endif
}