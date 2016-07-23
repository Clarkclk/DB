%{

#ifndef YYSTYPE
#define YYSTYPE void*
#endif

#include <cstdlib>
#include <cstdio>
#include "parser.hpp"
#include "../dbms.h"

#include <string>

using std::string;

void yyerror(char *);

%}

%%

\r|\n|\r\n              { /* Just ignore */    }
[ \t]+                  { /* Just ignore */    }
SELECT|(S|s)elect       { return SELECT; }
CREATE|(C|c)reate       { return CREATE; }
DROP|(D|d)rop           { return DROP; }
USE|(U|u)se             { return USE; }
SHOW|(S|s)how           { return SHOW; }
INSERT|(I|i)nsert       { return INSERT; }
INTO|(I|i)nto           { return INTO; }
VALUES|(V|v)alues       { return VALUES; }
DELETE|(D|d)elete       { return DELETE; }
FROM|(F|f)rom           { return FROM; }
WHERE|(W|w)here         { return WHERE; }
UPDATE|(U|u)pdate       { return UPDATE; }
SET|(S|s)et             { return SET; }
AND|(A|a)nd             { return AND; }
AS|(A|a)s               { return AS; }

DATABASE|(D|d)atabase   { return DATABASE; }
TABLE|(T|t)able         { return TABLE; }
NOT|(N|n)ot             { return NOT; }
NULL|(N|n)ull           { return NUL; }
PRIMARY|(P|p)rimary     { return PRIMARY; }
KEY|(K|k)ey             { return KEY; }
VARCHAR|(V|v)archar     { return VARCHAR; }
INT|(I|i)nt             { return INTT; }
FLOAT|(F|f)loat         { return FLOAT; }

"exit"                  { return EXIT; }

IS|(I|i)s               { yylval = (void *)SQL_OP_EQ; return SQL_OP; }
"="                     { yylval = (void *)SQL_OP_EQ; return SQL_OP; }
"!="                    { yylval = (void *)SQL_OP_NE; return SQL_OP; }
"<"                     { yylval = (void *)SQL_OP_LT; return SQL_OP; }
"<="                    { yylval = (void *)SQL_OP_LE; return SQL_OP; }
">"                     { yylval = (void *)SQL_OP_GT; return SQL_OP; }
">="                    { yylval = (void *)SQL_OP_GE; return SQL_OP; }

[0-9]+                  {
                            yylval = new int(atoi(yytext));
                            return INTEGER;
                        }
("+"|"-"|"*"|"/"|"%"|"="|"<"|">"|"."|","|";"|"!"|"("|")"|"["|"]"|"{"|"}"|"?"|":"|"'")	{
                            return *yytext;
                        }
[A-Za-z][_0-9A-Za-z]*   {
                            // printf("\"%s\"\n", yytext);
                            yylval = new string(yytext);
                            return IDENT;
                        }
'(\\.|[^'\\])*'         {
                            yylval = new string(yytext + 1);
                            ((string *)yylval)->pop_back();
                            return STRING;
                        }
`[A-Za-z][_0-9A-Za-z]*` {
                            // printf("\"%s\"\n", yytext);
                            yylval = new string(yytext + 1);
                            ((string *)yylval)->pop_back();
                            return IDENT;
                        }
.                       {
                            yyerror("invalid character: ");
                            fprintf(stderr, "'%c'\n", *yytext);
                        }

%%

int yywrap(void)
{
   return 1;
}

extern int yyparse();

#define LINE_BUF_SIZE 10000
char buf[LINE_BUF_SIZE];

DBMS dbms;

int main() {
    puts("Database terminal.");
    puts("DB course project. Jinxu Zhao, Likang Chen. v2.0");
    puts("");

    while (!dbms.quit) {
        dbms.showCurrentDB();

        fgets(buf, LINE_BUF_SIZE, stdin);
        yy_scan_string(buf);
        yyparse();
        yy_delete_buffer(YY_CURRENT_BUFFER);
    }
    return 0;
}

