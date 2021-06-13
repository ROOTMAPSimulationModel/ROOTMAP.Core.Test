%{
#ifdef WIN32
#pragma warning (disable : 4996)
#endif // #ifdef WIN32
#include <stdio.h>
#include <stdlib.h>
#include "InputSchemeParser.tab.h"
// NOTE that the "-1" for the ENDTOK index is based on InvalidBoxIndex
// we cannot #include ScoreboardBox.h here because it is a c file, which
// does not understand namespace declarations
%}

%%
set                     return SETTOK;
row|column              return ROWCOLTOK;
value                   return VALUETOK;
scoreboard              return SCOREBOARDTOK;
plane                   return PLANETOK;
box                     return BOXTOK;
boxes                   return BOXESTOK;
[XYZ]                   yylval.dimension=(yytext[0]); return DIMSPECVAL;
[-0-9][0-9]*            yylval.index=atol(yytext); return LONGVAL;
[-0-9][0-9.]+           yylval.value=atof(yytext); return DOUBLEVAL;
end                     yylval.index=-1; return ENDTOK;
\[                      return OPENARRAY;
\]                      return CLOSEARRAY;
=                       return EQUALS;
;                       return SEMICOLON;
,                       return COMMA;
\n                      /* ignore EOL */;
[ \t]+                  /* ignore whitespace */;
.                       printf("Don't recognise '%s'\n", yytext);
%%
