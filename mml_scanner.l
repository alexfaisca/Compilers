%option c++ prefix="mml_scanner_" outfile="mml_scanner.cpp"
%option stack noyywrap yylineno 8bit debug
%{
// make relevant includes before including the parser's tab file
#include <string>
#include <cstring>
#include <sstream>
#include <cdk/ast/sequence_node.h>
#include <cdk/ast/expression_node.h>
#include <cdk/ast/lvalue_node.h>
#include "mml_parser.tab.h"
#include <climits>
#include <float.h>

// don't change this
#define yyerror LexerError

static std::ostringstream strlit;
%}
SPACE     ([ \t\r]|\n)

%x X_STRING X_COMMENT X_BACKSLASH X_NULL
%%
  yydebug=1; set_debug(1);

">="                                return tGE;
"<="                                return tLE;
"=="                                return tEQ;
"!="                                return tNE;
">"                                 return *yytext;
"<"                                 return *yytext;
"~"                                 return *yytext;
"?"                                 return tADDRESSOF;
"sizeof"                            return tSIZEOF;
"&&"                                return tAND;
"||"                                return tOR;

"while"                             return tWHILE;
"if"                                return tIF;
"elif"                              return tELIF;
"else"                              return tELSE;
"!!"                                return tWRITELN;
"!"                                 return tWRITE;
"input"                             return tREAD;
"->"                                return tARROW;
"begin"                             return tBEGIN;
"end"                               return tEND;
"break"                              return tSTOP;
"next"                              return tNEXT;
"@"                                 return tRECURSION;
"return"                            return tRETURN;

"public"                            return tPUBLIC;
"forward"                           return tFORWARD;
"foreign"                           return tFOREIGN;
"auto"                              return tTYPE_AUTO;
"int"                               return tTYPE_INT;
"double"                            return tTYPE_REAL;
"string"                            return tTYPE_STRING;
"void"                              return tTYPE_VOID;
"null"                              return tNULLPTR;

[A-Za-z][A-Za-z0-9]*                yylval.s = new std::string(yytext); return tIDENTIFIER;

"\""                                yy_push_state(X_STRING); yylval.s = new std::string("");            
<X_STRING>"\""                      {
                                    yylval.s = new std::string(strlit.str());
                                    strlit.str("");
                                    yy_pop_state();
                                    return tSTRING;
                                    }                                    
<X_STRING>\\"0"[^0-7]               yy_push_state(X_NULL);
<X_STRING>\n                        yyerror("Lexical error: line feed inside string.");
<X_STRING>\\                        yy_push_state(X_BACKSLASH);                                    
<X_STRING>.                         strlit << *yytext;

<X_NULL>"\""                        {
                                    yy_pop_state();
                                    yylval.s = new std::string(strlit.str());
                                    strlit.str("");
                                    yy_pop_state();
                                    return tSTRING;
                                    }
<X_NULL>.                           ;

<X_BACKSLASH>t                      strlit << '\t'; yy_pop_state();
<X_BACKSLASH>n                      strlit << '\n'; yy_pop_state();
<X_BACKSLASH>r                      strlit << '\r'; yy_pop_state();
<X_BACKSLASH>\\                     strlit << '\\'; yy_pop_state();
<X_BACKSLASH>\"                     strlit << '\"'; yy_pop_state();
<X_BACKSLASH>[0-7]{1,3}             {
                                    if(strtoul(yytext, NULL, 8) > UCHAR_MAX) yyerror("Lexical error: octal number overflow.");
                                    strlit << (char)(unsigned char)strtoul(yytext, NULL, 8);
                                    yy_pop_state();
                                    }
<X_BACKSLASH>.                      strlit << *yytext; yy_pop_state();

"/*"                                yy_push_state(X_COMMENT);
<X_COMMENT>"/*"                     yy_push_state(X_COMMENT);
<X_COMMENT>"*/"                     yy_pop_state();
<X_COMMENT>.|\n                     ;

"//".*$                             ;

0|[1-9][0-9]*                       {
                                    if(strtoul(yytext, nullptr, 10) > INT_MAX) yyerror("Lexical error: decimal number overflow.");
                                    yylval.i = strtoul(yytext, nullptr, 10); return tINTEGER;
                                    }

0[0-7]+                             {
                                    if(strtoul(yytext, nullptr, 8) > INT_MAX) yyerror("Lexical error: octal number overflow.");
                                    yylval.i = strtoul(yytext, nullptr, 8); return tINTEGER;
                                    }
0[0-7]*[8-9][0-7]*                  yyerror("Lexical error: invalid octal number.");
                     
[-()<>=+*/%;{}.,]|"["|"]"           return *yytext;

([0-9]+\.[0-9]*|[0-9]*\.[0-9]+|[0-9]+)([Ee][-+]?[0-9]+)? {
                                                         if(strtod(yytext, nullptr) > DBL_MAX) yyerror("Lexical error: real number overflow.");
                                                         yylval.d = strtod(yytext, nullptr); return tREAL;
                                                         }

[[:space:]]                         ; /* ignore whitespace */

.                                   yyerror("Unknown character");

%%
