

%token SELECT CREATE DROP USE SHOW
%token INSERT INTO VALUES DELETE FROM WHERE UPDATE SET
%token AND SQL_OP AS

%token DATABASE TABLE
%token PRIMARY KEY NOT NUL VARCHAR INTT FLOAT

%token INTEGER IDENT STRING

%token EXIT

%left '+' '-'
%left '*' '/'

%{
#ifndef YYSTYPE
#define YYSTYPE void*
#endif

#include <stdio.h>
#include <string>
#include <deque>

#include "../dbms.h"

using std::string;
using std::deque;
using std::pair;

int yylex(void);
void yyerror(char *);

extern DBMS dbms;

#define REF(type,x) (*(type *)(x))
#define DEL(type,x) (delete (type *)(x))

typedef pair<RecordCondTbCol, DataNode> pRecordData;
typedef pair<string, string> pTableAlias;
typedef pair<RecordCondTbCol, string> pTableColAlias;

%}

%%

program  : CREATE DATABASE IDENT     { dbms.createDatabase(REF(string,$3)); DEL(string,$3); }
         | DROP DATABASE IDENT       { dbms.dropDatabase(REF(string,$3)); DEL(string,$3); }
         | USE DATABASE IDENT        { dbms.useDatabase(REF(string,$3)); DEL(string,$3); }
         | USE IDENT                 { dbms.useDatabase(REF(string,$2)); DEL(string,$2); }
         | SHOW DATABASE IDENT       { dbms.showDatabase(REF(string,$3)); DEL(string,$3); }
         | CREATE TABLE IDENT '(' AttrList ')' { dbms.createTable(REF(string,$3), REF(deque<TableColInfo>,$5)); DEL(string,$3); DEL(deque<TableColInfo>,$5); }
         | DROP TABLE IDENT          { dbms.dropTable(REF(string,$3)); DEL(string,$3); }
         | SHOW TABLE IDENT          { dbms.showTable(REF(string,$3)); DEL(string,$3); }
         | SHOW IDENT                { dbms.showData(REF(string,$2)); DEL(string,$2); }
         | INSERT INTO IDENT '(' IdentList ')' VALUES '(' ValueList ')' { dbms.insertInto(REF(string,$3), &REF(deque<string>,$5), REF(deque<DataNode>,$9)); DEL(string,$3); DEL(deque<string>,$5); DEL(deque<DataNode>,$9); }
         | INSERT INTO IDENT VALUES '(' ValueList ')' { dbms.insertInto(REF(string,$3), NULL, REF(deque<DataNode>,$6)); DEL(string,$3); DEL(deque<DataNode>,$6); }
         | DELETE FROM IDENT WHERE whereClauses { dbms.deleteFrom(REF(string,$3), REF(deque<RecordCondHelper>,$5)); DEL(string,$3); DEL(deque<RecordCondition>,$5); }
         | SELECT tableColAliasList FROM tableAliasList WHERE whereClauses { dbms.selectFrom(REF(deque<pTableColAlias>,$2), REF(deque<pTableAlias>,$4), REF(deque<RecordCondHelper>,$6)); DEL(deque<pTableColAlias>,$2); DEL(deque<pTableAlias>,$4); DEL(deque<RecordCondHelper>,$6); }
         | UPDATE IDENT SET equList WHERE whereClauses { dbms.update(REF(string,$2), REF(deque<pRecordData>,$4), REF(deque<RecordCondHelper>,$6)); DEL(string,$2); DEL(deque<pRecordData>,$4); DEL(deque<RecordCondHelper>,$6); }
         | expr                      { printf("%d\n", REF(int,$1)); DEL(int,$1); }
         | EXIT                      { dbms.exit(); }
         ;

equList : equ ',' equList { $$ = $3; REF(deque<pRecordData>,$3).push_front(REF(pRecordData,$1)); DEL(pRecordData,$1); }
        | equ             { $$ = new deque<pRecordData>(); REF(deque<pRecordData>, $$).push_back(REF(pRecordData,$1)); DEL(pRecordData,$1); }

equ : tableCol SQL_OP Value { if ($2 != (void *)SQL_OP_EQ) yyerror(NULL); $$ = new pair<RecordCondTbCol, DataNode>(REF(RecordCondTbCol,$1), REF(DataNode,$3)); }

whereClauses : whereClause AND whereClauses     { $$ = $3; REF(deque<RecordCondHelper>,$3).push_front(REF(RecordCondHelper,$1)); DEL(RecordCondHelper,$1); }
             | whereClause                      { $$ = new deque<RecordCondHelper>(); REF(deque<RecordCondHelper>,$$).push_back(REF(RecordCondHelper,$1)); DEL(RecordCondHelper,$1); }

whereClause : tableCol SQL_OP tableCol              { $$ = new RecordCondHelper((SQL_Operators)(long long)$2, REF(RecordCondTbCol, $1), REF(RecordCondTbCol, $3)); DEL(RecordCondTbCol, $1); DEL(RecordCondTbCol, $3); }
            | tableCol SQL_OP Value                 { $$ = new RecordCondHelper((SQL_Operators)(long long)$2, REF(RecordCondTbCol, $1), REF(DataNode,$3)); DEL(RecordCondTbCol, $1); DEL(DataNode,$3); }
            | Value SQL_OP tableCol                 { $$ = new RecordCondHelper((SQL_Operators)(long long)$2, REF(DataNode,$1), REF(RecordCondTbCol, $3)); DEL(DataNode,$1); DEL(RecordCondTbCol, $3); }

tableAliasList : tableAlias ',' tableAliasList  { $$ = $3; REF(deque<pTableAlias>,$3).push_front(REF(pTableAlias,$1)); DEL(pTableAlias,$1); }
               | tableAlias                     { $$ = new deque<pTableAlias>(); REF(deque<pTableAlias>,$$).push_back(REF(pTableAlias,$1)); DEL(pTableAlias,$1); }

tableAlias : IDENT            { $$ = new pair<string, string>(REF(string,$1), ""); DEL(string,$1); }
           | IDENT AS IDENT   { $$ = new pair<string, string>(REF(string,$1), REF(string,$3)); DEL(string,$1); DEL(string,$3); }

IdentList : IDENT ',' IdentList     { $$ = $3; REF(deque<string>,$3).push_front(REF(string,$1)); DEL(string,$1); }
          | IDENT                   { $$ = new deque<string>(); REF(deque<string>,$$).push_back(REF(string,$1)); DEL(string,$1); }
          ;

tableColAliasList : tableColAlias ',' tableColAliasList  { $$ = $3; REF(deque<pTableColAlias>,$3).push_front(REF(pTableColAlias,$1)); DEL(pTableColAlias,$1); }
                  | tableColAlias                        { $$ = new deque<pTableColAlias>(); REF(deque<pTableColAlias>,$$).push_back(REF(pTableColAlias,$1)); DEL(pTableColAlias,$1); }
                  | '*'                                  { $$ = new deque<pTableColAlias>(); }
                    ;

tableColAlias : tableCol                  { $$ = new pair<RecordCondTbCol, string>(REF(RecordCondTbCol,$1), ""); DEL(RecordCondTbCol,$1); }
              | tableCol AS IDENT         { $$ = new pair<RecordCondTbCol, string>(REF(RecordCondTbCol,$1), REF(string,$3)); DEL(RecordCondTbCol,$1); DEL(string,$3); }

tableCol : IDENT '.' IDENT                      { $$ = new RecordCondTbCol(REF(string,$1), REF(string,$3)); DEL(string, $1); DEL(string, $3); }
         | IDENT                                { $$ = new RecordCondTbCol(REF(string,$1)); DEL(string, $1); }

ValueList : Value ',' ValueList { $$ = $3; REF(deque<DataNode>,$3).push_front(REF(DataNode,$1)); DEL(DataNode,$1); }
          | Value { $$ = new deque<DataNode>(); REF(deque<DataNode>,$$).push_back(REF(DataNode,$1)); DEL(DataNode,$1); }
          ;

Value    : INTEGER                       { $$ = new DataNode(REF(int,$1)); DEL(int,$1); }
         | STRING                        { $$ = new DataNode(REF(string,$1).c_str()); DEL(string,$1); }
         | NUL                           { $$ = new DataNode(); }
         ;

AttrList : Attr ',' AttrList   { $$ = $3; REF(deque<TableColInfo>,$3).push_front(REF(TableColInfo,$1)); DEL(TableColInfo,$1); }
         | Attr                { $$ = new deque<TableColInfo>(); REF(deque<TableColInfo>,$$).push_back(REF(TableColInfo,$1)); DEL(TableColInfo,$1); }
         ;

Attr     : IDENT ColType                 { $$ = new TableColInfo(REF(int,$2) & 0xFFFF, REF(unsigned,$2) >> 16, REF(string,$1).c_str());
                                           DEL(string, $1); DEL(int,$2); }
         | IDENT ColType NOT NUL         { $$ = new TableColInfo(REF(int,$2) & 0xFFFF, REF(unsigned,$2) >> 16, REF(string,$1).c_str(), TableColInfo::NOTNULL);
                                           DEL(string, $1); DEL(int,$2); }
         | PRIMARY KEY '(' IDENT ')'     { $$ = new TableColInfo(-1, 0, REF(string,$4).c_str(), TableColInfo::PRIMARYKEY); DEL(string,$4); }
         ;

ColType  : VARCHAR '(' INTEGER ')'       { $$ = new int(DataTypeLabel::TYPE_COL_VARCHAR + (REF(int,$3) << 16)); DEL(int,$3); }
         | INTT                          { $$ = new int(DataTypeLabel::TYPE_COL_INT + (4 << 16)); }
         | FLOAT                         { $$ = new int(DataTypeLabel::TYPE_COL_FLOAT + (8 << 16)); }
         ;

expr:
         INTEGER                  { $$ = $1; }
        | expr '+' expr           { $$ = new int(REF(int,$1) + REF(int,$3)); DEL(int,$1); DEL(int,$3); }
        | expr '-' expr           { $$ = new int(REF(int,$1) - REF(int,$3)); DEL(int,$1); DEL(int,$3); }
        ;

%%

void yyerror(char *s) {
    fprintf(stderr, "*** Error: %s\n", s);
}

