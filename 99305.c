JsVar *jsvAsArrayIndex(JsVar *index) {
  if (jsvIsSimpleInt(index)) {
    return jsvLockAgain(index); // we're ok!
  } else if (jsvIsString(index)) {
    /* Index filtering (bug #19) - if we have an array index A that is:
     is_string(A) && int_to_string(string_to_int(A)) == A
     then convert it to an integer. Shouldn't be too nasty for performance
     as we only do this when accessing an array with a string */
    if (jsvIsStringNumericStrict(index)) {
      JsVar *i = jsvNewFromInteger(jsvGetInteger(index));
      JsVar *is = jsvAsString(i, false);
      if (jsvCompareString(index,is,0,0,false)==0) {
        jsvUnLock(is);
        return i;
      } else {
        jsvUnLock2(i,is);
      }
    }
  } else if (jsvIsFloat(index)) {
    JsVarFloat v = jsvGetFloat(index);
    JsVarInt vi = jsvGetInteger(index);
    if (v == vi) return jsvNewFromInteger(vi);
  }

  return jsvAsString(index, false);
}
