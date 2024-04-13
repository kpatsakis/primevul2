JsVarFloat stringToFloatWithRadix(
    const char *s, //!< The string to be converted to a float
  	int forceRadix, //!< The radix of the string data, or 0 to guess
  	const char **endOfFloat //!<  If nonzero, this is set to the point at which the float finished in the string
  ) {
  while (isWhitespace(*s)) s++;

  bool isNegated = false;
  if (*s == '-') {
    isNegated = true;
    s++;
  } else if (*s == '+') {
    s++;
  }

  const char *numberStart = s;
  if (endOfFloat) (*endOfFloat)=s;

  int radix = getRadix(&s, forceRadix, 0);
  if (!radix) return NAN;


  JsVarFloat v = 0;
  JsVarFloat mul = 0.1;

  while (*s) {
    int digit = chtod(*s);
    if (digit<0 || digit>=radix)
      break;
    v = (v*radix) + digit;
    s++;
  }

  if (radix == 10) {
    if (*s == '.') {
      s++; // skip .

      while (*s) {
        if (*s >= '0' && *s <= '9')
          v += mul*(*s - '0');
        else break;
        mul /= 10;
        s++;
      }
    }

    if (*s == 'e' || *s == 'E') {
      s++;  // skip E
      bool isENegated = false;
      if (*s == '-' || *s == '+') {
        isENegated = *s=='-';
        s++;
      }
      int e = 0;
      while (*s) {
        if (*s >= '0' && *s <= '9')
          e = (e*10) + (*s - '0');
        else break;
        s++;
      }
      if (isENegated) e=-e;
      while (e>0) {
        v*=10;
        e--;
      }
      while (e<0) {
        v/=10;
        e++;
      }
    }
  }

  if (endOfFloat) (*endOfFloat)=s;
  if (numberStart==s || (numberStart[0]=='.' && numberStart[1]==0)) return NAN;

  if (isNegated) return -v;
  return v;
}
