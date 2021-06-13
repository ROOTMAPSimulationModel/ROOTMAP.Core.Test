%{
#define _CRT_NONSTDC_NO_DEPRECATE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */

#ifdef WIN32
#pragma warning (disable : 4786)
#pragma warning (disable : 4273)
#endif // #ifdef WIN32

#if defined __cplusplus
extern "C"
{
#endif
extern void onScoreboardDeclaration();
extern void onPlaneDeclaration();
extern void onRowColumnDeclaration();
extern void onBoxDeclaration();
extern void onBoxesDeclaration();

extern void onDimensionSpec(char c, long index);
extern void onValueSpec(double value);
extern void onArraySpec(long x, long y, long z);

extern void onStatementEnd();
extern void onParseStart();
extern void onParseEnd();
#if defined __cplusplus
}
#endif

extern int yylex();
int yydebug=0;

void yyerror(const char *str)
{
	fprintf(stderr,"error: %s\n",str);
}

int yywrap()
{
	return 1;
}

extern FILE *yyin;

void ParseScheme(const char * scheme_str)
{
    FILE * my_tmp_file = 0;

    my_tmp_file = fopen("./inputschemeparse.tmp","w+tD");
    yyin = my_tmp_file;
    yydebug=0;

    fwrite(scheme_str,1,strlen(scheme_str),my_tmp_file);
    fflush(my_tmp_file);
    fseek(my_tmp_file,0,SEEK_SET);
    
    onParseStart();

	yyparse();
	
	onParseEnd();

    fclose(my_tmp_file);
}

%}

%token SETTOK ROWCOLTOK VALUETOK SCOREBOARDTOK PLANETOK BOXTOK BOXESTOK EQUALS SEMICOLON COMMA OPENARRAY CLOSEARRAY

%union
{
    long    index;
    double  value;
    char    dimension;
}

%token <index>      LONGVAL
%token <value>      DOUBLEVAL
%token <dimension>  DIMSPECVAL
%token <index>      ENDTOK
%type  <double>     value_assignment
%type  <index>      index_assignment

%%


statements:
	| statements statement
	;

statement: scoreboarddecl | planedecl | rowcoldecl | boxdecl | boxesdecl

scoreboarddecl:
    SETTOK SCOREBOARDTOK value_assignment SEMICOLON
    {
        onScoreboardDeclaration();
        onStatementEnd();
    }
    ;

planedecl:
    SETTOK PLANETOK dimension_assignment value_assignment SEMICOLON
    {
        onPlaneDeclaration();
        onStatementEnd();
    }
    ;

rowcoldecl:
    SETTOK ROWCOLTOK dimension_assignment dimension_assignment value_assignment SEMICOLON
    {
        onRowColumnDeclaration();
        onStatementEnd();
    }
    ;

boxdecl:
    SETTOK BOXTOK box_assignment value_assignment SEMICOLON
    {
        onBoxDeclaration();
        onStatementEnd();
    }
    ;

boxesdecl:
    SETTOK BOXESTOK box_assignment box_assignment value_assignment SEMICOLON
    {
        onBoxesDeclaration();
        onStatementEnd();
    }
    ;
    
box_assignment:
    OPENARRAY index_assignment COMMA index_assignment COMMA index_assignment CLOSEARRAY
    {
        onArraySpec($2,$4,$6);
    }
    ;

index_assignment:
    LONGVAL | ENDTOK
    {
        $$ = $1;
    }
    ;
    
value_assignment:
    VALUETOK EQUALS DOUBLEVAL
    {
        onValueSpec($3);
    }
    ;

dimension_assignment:
    DIMSPECVAL EQUALS LONGVAL
    {
        onDimensionSpec($1, $3);
    }
    ;

