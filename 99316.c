JsVar *jsvMakeIntoVariableName(JsVar *var, JsVar *valueOrZero) {
  if (!var) return 0;
  assert(jsvGetRefs(var)==0); // make sure it's unused
  assert(jsvIsSimpleInt(var) || jsvIsString(var));
  JsVarFlags varType = (var->flags & JSV_VARTYPEMASK);
  if (varType==JSV_INTEGER) {
    int t = JSV_NAME_INT;
    if ((jsvIsInt(valueOrZero) || jsvIsBoolean(valueOrZero)) && !jsvIsPin(valueOrZero)) {
      JsVarInt v = valueOrZero->varData.integer;
      if (v>=JSVARREF_MIN && v<=JSVARREF_MAX) {
        t = jsvIsInt(valueOrZero) ? JSV_NAME_INT_INT : JSV_NAME_INT_BOOL;
        jsvSetFirstChild(var, (JsVarRef)v);
        valueOrZero = 0;
      }
    }
    var->flags = (JsVarFlags)(var->flags & ~JSV_VARTYPEMASK) | t;
  } else if (varType>=_JSV_STRING_START && varType<=_JSV_STRING_END) {
    if (jsvGetCharactersInVar(var) > JSVAR_DATA_STRING_NAME_LEN) {
      /* Argh. String is too large to fit in a JSV_NAME! We must chomp make
       * new STRINGEXTs to put the data in
       */
      JsvStringIterator it;
      jsvStringIteratorNew(&it, var, JSVAR_DATA_STRING_NAME_LEN);
      JsVar *startExt = jsvNewWithFlags(JSV_STRING_EXT_0);
      JsVar *ext = jsvLockAgainSafe(startExt);
      size_t nChars = 0;
      while (ext && jsvStringIteratorHasChar(&it)) {
        if (nChars >= JSVAR_DATA_STRING_MAX_LEN) {
          jsvSetCharactersInVar(ext, nChars);
          JsVar *ext2 = jsvNewWithFlags(JSV_STRING_EXT_0);
          if (ext2) {
            jsvSetLastChild(ext, jsvGetRef(ext2));
          }
          jsvUnLock(ext);
          ext = ext2;
          nChars = 0;
        }
        ext->varData.str[nChars++] = jsvStringIteratorGetChar(&it);
        jsvStringIteratorNext(&it);
      }
      jsvStringIteratorFree(&it);
      if (ext) {
        jsvSetCharactersInVar(ext, nChars);
        jsvUnLock(ext);
      }
      jsvSetCharactersInVar(var, JSVAR_DATA_STRING_NAME_LEN);
      JsVarRef oldRef = jsvGetLastChild(var);
      while (oldRef) {
        JsVar *v = jsvGetAddressOf(oldRef);
        oldRef = jsvGetLastChild(v);
        jsvFreePtrInternal(v);
      }
      jsvSetLastChild(var, jsvGetRef(startExt));
      jsvSetNextSibling(var, 0);
      jsvSetPrevSibling(var, 0);
      jsvSetFirstChild(var, 0);
      jsvUnLock(startExt);
    }

    size_t t = JSV_NAME_STRING_0;
    if (jsvIsInt(valueOrZero) && !jsvIsPin(valueOrZero)) {
      JsVarInt v = valueOrZero->varData.integer;
      if (v>=JSVARREF_MIN && v<=JSVARREF_MAX) {
        t = JSV_NAME_STRING_INT_0;
        jsvSetFirstChild(var, (JsVarRef)v);
        valueOrZero = 0;
      }
    } else
      jsvSetFirstChild(var, 0);
    var->flags = (var->flags & (JsVarFlags)~JSV_VARTYPEMASK) | (t+jsvGetCharactersInVar(var));
  } else assert(0);

  if (valueOrZero)
    jsvSetFirstChild(var, jsvGetRef(jsvRef(valueOrZero)));
  return var;
}
