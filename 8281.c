void HierarchicalBitmapRequester::Push8Lines(UBYTE c)
{
  int cnt;
  ULONG y = m_pulY[c];
  //
  for(cnt = 0;cnt < 8 && y < m_pulHeight[c];cnt++) {
    assert(m_ppEncodingMCU[cnt | (c << 3)]);
    m_pLargestScale->PushLine(m_ppEncodingMCU[cnt | (c << 3)],c);
    m_ppEncodingMCU[cnt | (c << 3)] = NULL;
    y++;
  }

  m_pulY[c] = y;
}