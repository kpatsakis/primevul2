void jsvAppendString(JsVar *var, const char *str) {
  assert(jsvIsString(var));
  JsvStringIterator dst;
  jsvStringIteratorNew(&dst, var, 0);
  jsvStringIteratorGotoEnd(&dst);
  /* This isn't as fast as something single-purpose, but it's not that bad,
   * and is less likely to break :) */
  while (*str)
    jsvStringIteratorAppend(&dst, *(str++));
  jsvStringIteratorFree(&dst);
}
