#ifndef DBMS_H
#define DBMS_H

#include <string>
#include <deque>
#include <vector>

#include "database.h"
#include "table.h"
#include "datanode.h"

using std::string;
using std::deque;
using std::vector;

class DBMS
{
    Database *currentDB = NULL;

    void exitCurrent();
public:
    DBMS();
    ~DBMS();

    char quit = 0;

    void showCurrentDB();

    void createDatabase(string dbName);
    void dropDatabase(string dbName);
    void useDatabase(string dbName);
    void showDatabase(string dbName);

    void createTable(string tableName, deque<TableColInfo> colInfo);
    void dropTable(string tableName);
    void showTable(string tableName);
    void showData(string tableName);

    void insertInto(string tableName, deque<string> *colNames, deque<DataNode> &values);
    void deleteFrom(string tableName, deque<RecordCondHelper> &conditions);
    void selectFrom(deque<pair<RecordCondTbCol, string>> &colNames, deque<pair<string, string> > &tableNames, deque<RecordCondHelper> &conditions);
    void update(string tableName, deque<pair<RecordCondTbCol, DataNode>> &setCols, deque<RecordCondHelper> &conditions);

    void exit();
};

#endif // DBMS_H
