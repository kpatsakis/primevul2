void HierarchicalBitmapRequester::FetchRegion(LONG x,const struct Line *const *line,LONG *buffer)
{
  int cnt = 8;
  do {
    if (*line) 
      memcpy(buffer,(*line)->m_pData + (x << 3),8 * sizeof(LONG));
    buffer += 8;
    line++;
  } while(--cnt);
}