void HierarchicalBitmapRequester::Pull8Lines(UBYTE c)
{ 
 int cnt;
  ULONG y = m_pulY[c];
  //
  // Allocate a line block from the encoding line adapter.
  for(cnt = 0;cnt < 8 && y < m_pulHeight[c];cnt++) {
    assert(m_ppDecodingMCU[cnt | (c << 3)] == NULL);
    m_ppDecodingMCU[cnt | (c << 3)] = m_pLargestScale->GetNextLine(c);
    y++;
  }
}