void HierarchicalBitmapRequester::DefineRegion(LONG x,const struct Line *const *line,
                                               const LONG *buffer,UBYTE comp)
{
  int cnt = 8;
  
  assert(comp < m_ucCount);
  NOREF(comp);

  x <<= 3;
  
  do {
    if (*line) memcpy((*line)->m_pData + x,buffer,8 * sizeof(LONG));
    buffer += 8;
    line++;
  } while(--cnt);
}