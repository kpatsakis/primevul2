 void jslTokenAsString(int token, char *str, size_t len) {
  assert(len>28); // size of largest string
   if (token>32 && token<128) {
     assert(len>=4);
    str[0] = '\'';
    str[1] = (char)token;
    str[2] = '\'';
    str[3] = 0;
     return;
   }
 

   switch (token) {
  case LEX_EOF : strcpy(str, "EOF"); return;
  case LEX_ID : strcpy(str, "ID"); return;
  case LEX_INT : strcpy(str, "INT"); return;
  case LEX_FLOAT : strcpy(str, "FLOAT"); return;
  case LEX_STR : strcpy(str, "STRING"); return;
  case LEX_UNFINISHED_STR : strcpy(str, "UNFINISHED STRING"); return;
  case LEX_TEMPLATE_LITERAL : strcpy(str, "TEMPLATE LITERAL"); return;
  case LEX_UNFINISHED_TEMPLATE_LITERAL : strcpy(str, "UNFINISHED TEMPLATE LITERAL"); return;
  case LEX_REGEX : strcpy(str, "REGEX"); return;
  case LEX_UNFINISHED_REGEX : strcpy(str, "UNFINISHED REGEX"); return;
  case LEX_UNFINISHED_COMMENT : strcpy(str, "UNFINISHED COMMENT"); return;
   }
   if (token>=_LEX_OPERATOR_START && token<_LEX_R_LIST_END) {
     const char tokenNames[] =
        /* LEX_EQUAL      :   */ "==\0"
        /* LEX_TYPEEQUAL  :   */ "===\0"
        /* LEX_NEQUAL     :   */ "!=\0"
        /* LEX_NTYPEEQUAL :   */ "!==\0"
        /* LEX_LEQUAL    :    */ "<=\0"
        /* LEX_LSHIFT     :   */ "<<\0"
        /* LEX_LSHIFTEQUAL :  */ "<<=\0"
        /* LEX_GEQUAL      :  */ ">=\0"
        /* LEX_RSHIFT      :  */ ">>\0"
        /* LEX_RSHIFTUNSIGNED */ ">>>\0"
        /* LEX_RSHIFTEQUAL :  */ ">>=\0"
        /* LEX_RSHIFTUNSIGNEDEQUAL */ ">>>=\0"
        /* LEX_PLUSEQUAL   :  */ "+=\0"
        /* LEX_MINUSEQUAL  :  */ "-=\0"
        /* LEX_PLUSPLUS :     */ "++\0"
        /* LEX_MINUSMINUS     */ "--\0"
        /* LEX_MULEQUAL :     */ "*=\0"
        /* LEX_DIVEQUAL :     */ "/=\0"
        /* LEX_MODEQUAL :     */ "%=\0"
        /* LEX_ANDEQUAL :     */ "&=\0"
        /* LEX_ANDAND :       */ "&&\0"
        /* LEX_OREQUAL :      */ "|=\0"
        /* LEX_OROR :         */ "||\0"
        /* LEX_XOREQUAL :     */ "^=\0"
        /* LEX_ARROW_FUNCTION */ "=>\0"

        /*LEX_R_IF :       */ "if\0"
        /*LEX_R_ELSE :     */ "else\0"
        /*LEX_R_DO :       */ "do\0"
        /*LEX_R_WHILE :    */ "while\0"
        /*LEX_R_FOR :      */ "for\0"
        /*LEX_R_BREAK :    */ "return\0"
        /*LEX_R_CONTINUE   */ "continue\0"
        /*LEX_R_FUNCTION   */ "function\0"
        /*LEX_R_RETURN     */ "return\0"
        /*LEX_R_VAR :      */ "var\0"
        /*LEX_R_LET :      */ "let\0"
        /*LEX_R_CONST :    */ "const\0"
        /*LEX_R_THIS :     */ "this\0"
        /*LEX_R_THROW :    */ "throw\0"
        /*LEX_R_TRY :      */ "try\0"
        /*LEX_R_CATCH :    */ "catch\0"
        /*LEX_R_FINALLY :  */ "finally\0"
        /*LEX_R_TRUE :     */ "true\0"
        /*LEX_R_FALSE :    */ "false\0"
        /*LEX_R_NULL :     */ "null\0"
        /*LEX_R_UNDEFINED  */ "undefined\0"
        /*LEX_R_NEW :      */ "new\0"
        /*LEX_R_IN :       */ "in\0"
        /*LEX_R_INSTANCEOF */ "instanceof\0"
        /*LEX_R_SWITCH     */ "switch\0"
        /*LEX_R_CASE       */ "case\0"
        /*LEX_R_DEFAULT    */ "default\0"
        /*LEX_R_DELETE     */ "delete\0"
        /*LEX_R_TYPEOF :   */ "typeof\0"
        /*LEX_R_VOID :     */ "void\0"
        /*LEX_R_DEBUGGER : */ "debugger\0"
        /*LEX_R_CLASS :    */ "class\0"
        /*LEX_R_EXTENDS :  */ "extends\0"
        /*LEX_R_SUPER :  */   "super\0"
        /*LEX_R_STATIC :   */ "static\0"
        ;
    unsigned int p = 0;
    int n = token-_LEX_OPERATOR_START;
    while (n>0 && p<sizeof(tokenNames)) {
      while (tokenNames[p] && p<sizeof(tokenNames)) p++;
      p++; // skip the zero
       n--; // next token
     }
     assert(n==0);
    strcpy(str, &tokenNames[p]);
     return;
   }
 
   espruino_snprintf(str, len, "?[%d]", token);
 }
