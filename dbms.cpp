#include <cstdio>
#include <cstdlib>
#include <map>
#include <set>

#include "dbms.h"

using std::string;
using std::map;
using std::set;

#define ASSERT_CUR_DATABASE() { if (currentDB == NULL) { puts("** Error: No database is currently opened."); return; } }
#define GET_AND_ASSERT_TABLE(p,name) { p = currentDB->loadTable(tableName.c_str());\
    if (!p) { printf("** Error: Table '%s' not found.\n", tableName.c_str()); return; } }

DBMS::DBMS()
{
}

DBMS::~DBMS()
{
    exitCurrent();
}

void DBMS::exitCurrent() {
    if (currentDB) {
        delete currentDB;
        currentDB = NULL;
    }
}

void DBMS::showCurrentDB() {
    if (currentDB)
        printf("%s", currentDB->name);
    printf("> ");
}

void DBMS::createDatabase(string dbName)
{
    exitCurrent();
    string DBFileName = dbName; DBFileName.append(".db");
    Database *tempDB = Database::loadDatabase(DBFileName.c_str());
    if (tempDB) {
        printf("** Error: Database File '%s' already exist!\n", DBFileName.c_str());
    } else {
        tempDB = Database::createDatabase(DBFileName.c_str(), dbName.c_str());
        printf("* Success: Database '%s' created!\n", tempDB->name);
    }
    delete tempDB;
}

void DBMS::dropDatabase(string dbName) {
    exitCurrent();
    string DBFileName = dbName; DBFileName.append(".db");
    if (remove(DBFileName.c_str()) != 0) {
        printf("** Error: Cannot drop database '%s'. File '%s' cannot be removed.\n", DBFileName.c_str(), dbName.c_str());
    } else {
        printf("* Success: Database '%s' dropped!\n", dbName.c_str());
    }
}

void DBMS::useDatabase(string dbName) {
    exitCurrent();
    string DBFileName = dbName; DBFileName.append(".db");
    currentDB = Database::loadDatabase(DBFileName.c_str());
    if (!currentDB) {
        printf("** Error: Database '%s' does not exist!\n", dbName.c_str());
    } else {
        printf("* Success: Database '%s' loaded (size = %d page(s)).\n", currentDB->name, currentDB->curPage);
    }
}

void DBMS::showDatabase(string dbName) {
    exitCurrent();
    string DBFileName = dbName; DBFileName.append(".db");
    currentDB = Database::loadDatabase(DBFileName.c_str());
    if (!currentDB) {
        printf("** Error: Database '%s' does not exist!\n", dbName.c_str());
    } else {
        printf("Database '%s':\n", dbName.c_str());
        for (int i = 0; i < currentDB->tableCnt; i++)
            printf("\t%5s\t@page:%3d\n", currentDB->tableName[i], currentDB->tablePage[i]);
    }
}

void DBMS::createTable(string tableName, deque<TableColInfo> colInfo) {
    ASSERT_CUR_DATABASE();

    vector<TableColInfo> realColInfo;
    for (TableColInfo &info : colInfo) {
        if (info.colType == -1) { // primary key declaration
            bool processed = false;
            for (TableColInfo &i : realColInfo)
                if (strcmp(i.colName, info.colName) == 0) {
                    i.specialType |= info.specialType;
                    processed = true;
                    break;
                }
            if (!processed) {
                printf("** Error: Cannot find column '%s' when declaring primary key.\n", info.colName);
                return;
            }
        } else { // just push back the info
            realColInfo.push_back(info);
        }
    }
    Table *table = currentDB->createTable(tableName.c_str(), realColInfo);
    if (table) {
        printf("* Success: Table '%s' created.\n", tableName.c_str());
        delete table;
    }
    else
        printf("** Error: Table '%s' already exist.\n", tableName.c_str());
}

void DBMS::dropTable(string tableName) {
    ASSERT_CUR_DATABASE();

    if (currentDB->dropTable(tableName.c_str()))
        printf("* Success: Table '%s' dropped.\n", tableName.c_str());
    else
        printf("** Error: Table '%s' not found.\n", tableName.c_str());
}

void DBMS::showTable(string tableName) {
    ASSERT_CUR_DATABASE();

    Table *table = currentDB->loadTable(tableName.c_str());
    if (table) {
        table->showInfo();
        delete table;
    } else
        printf("** Error: Table '%s' not found.\n", tableName.c_str());
}

void DBMS::showData(string tableName) {
    ASSERT_CUR_DATABASE();

    Table *table = currentDB->loadTable(tableName.c_str());
    if (table) {
        table->showData();
        delete table;
    } else
        printf("** Error: Table '%s' not found.\n", tableName.c_str());
}

void DBMS::insertInto(string tableName, deque<string> *colNames, deque<DataNode> &values) {
    ASSERT_CUR_DATABASE();

    Table *table;
    GET_AND_ASSERT_TABLE(table, tableName.c_str());
    vector<shared_ptr<DataNode>> insertValues(table->colCnt, make_shared<DataNode>());

    if (!colNames) {
        if (values.size() != table->colCnt) {
            printf("** Error: #Values not match with #Columns.\n");
            return;
        }
        int i = 0;
        for (DataNode &data : values) {
            data.length = table->colLength[i];
            insertValues[i++] = make_shared<DataNode>(data);
        }
    } else { // we need to check the existance of the columns
        if (values.size() != colNames->size()) {
            printf("** Error: #Values not match with #Columns.\n");
            return;
        }
        for (string &col : *colNames) {
            int i;
            for (i = 0; i < table->colCnt; i++)
                if (strcmp(col.c_str(), table->colName[i]) == 0)
                    break;
            if (i == table->colCnt) { // this column does not exist!
                printf("** Error: Table '%s' does not exist column '%s'\n", tableName.c_str(), col.c_str());
                return;
            }

            DataNode &data = values.front();
            data.length = table->colLength[i];
            insertValues[i] = make_shared<DataNode>(data); values.pop_front();
        }
    }
    for (int i = 0; i < table->colCnt; i++) {
        shared_ptr<DataNode> pData = insertValues[i];
        if (!(table->colType[i] == pData->type ||
                (((table->colSpecialType[i] & TableColInfo::NOTNULL) == 0) && pData->type == TYPE_COL_NULL))) {
            printf("** Error: value type not match with column type on column '%s'.\n", table->colName[i]);
            return;
        }
    }

    if (!table->insertRecord(insertValues)) return;
    printf("* Success: Inserted one record to '%s'.\n", tableName.c_str());
}

void DBMS::deleteFrom(string tableName, deque<RecordCondHelper> &conditions) {
    ASSERT_CUR_DATABASE();

    Table *table;
    GET_AND_ASSERT_TABLE(table, tableName.c_str());
    // preprocess the conditions
    map<string, map<string, pair<Table *, int>>> colInd;
    for (int i = 0; i < table->colCnt; i++) {
        map<string, pair<Table *, int>> elem;
        elem[string(table->name)] = make_pair(table, i);
        colInd[string(table->colName[i])] = elem;
    }
    vector<RecordCondition> queryConditions;
    for (RecordCondHelper &condHelper : conditions) {
        bool fail = false;
        queryConditions.push_back(condHelper.convertToCondition(colInd, fail));
        if (fail) return;
    }

    vector<shared_ptr<Record>> result = table->queryWithCondition(queryConditions);
    int rmCnt = 0;
    for (shared_ptr<Record> rec : result) {
        rmCnt++;
        rec->remove();
    }
    printf("* Success: %d records deleted from table '%s'.\n", rmCnt, table->name);
}

void DBMS::selectFrom(deque<pair<RecordCondTbCol, string>> &colNames, deque<pair<string, string>> &tableNames, deque<RecordCondHelper> &conditions) {
    ASSERT_CUR_DATABASE();

    map<string, Table *> tables;
    map<string, map<string, pair<Table *, int>>> colInd;
    vector<string> resColNames;
    for (const pair<string, string> &p : tableNames) {
        const string &tbName = p.first;
        string curName = p.second == "" ? tbName : p.second;
        Table *table = currentDB->loadTable(tbName.c_str());
        if (!table) {
            printf("** Error: table '%s' not found!\n", tbName.c_str());
            for (pair<string, Table *> p : tables)
                delete p.second;
            return;
        }
        if (tables.find(curName) != tables.end()) { // duplicate table names!
            printf("** Error: Duplicate table name '%s'.", curName.c_str());
            for (pair<string, Table *> p : tables)
                delete p.second;
            return;
        }
        tables[curName] = table;

        for (int i = 0; i < table->colCnt; i++) {
            auto result = colInd.find(string(table->colName[i]));
            if (result == colInd.end()) {
                map<string, pair<Table *, int>> elem;
                elem[curName] = make_pair(table, i);
                colInd[string(table->colName[i])] = elem;
            } else {
                result->second[curName] = make_pair(table, i);
            }
        }
    }

    vector<RecordColRef> colRef;
    if (colNames.size() == 0) { // SELECT * FROM ...
        for (const pair<const string, map<string, pair<Table *, int>>> &ncP : colInd) {
            bool isOnly = true;
            if (ncP.second.size() > 1) isOnly = false;
            for (const pair<const string, pair<Table *, int>> &tP : ncP.second) {
                colRef.push_back(RecordColRef(tP.second.first, tP.second.second));
                if (isOnly)
                    resColNames.push_back(ncP.first);
                else
                    resColNames.push_back(tP.first + "." + ncP.first);
            }
        }
    }
    else {
        for (pair<RecordCondTbCol, string> &p : colNames) {
            RecordCondTbCol &tbCol = p.first;
            bool fail = false;
            colRef.push_back(tbCol.convertToColRef(colInd, fail));
            if (fail) return;

            if (p.second != "")
                resColNames.push_back(p.second);
            else {
                if (tbCol.table != "")
                    resColNames.push_back(tbCol.table + "." + tbCol.col);
                else
                    resColNames.push_back(tbCol.col);
            }
        }
    }
    vector<RecordCondition> queryConditions;
    for (RecordCondHelper &condHelper : conditions) {
        bool fail = false;
        queryConditions.push_back(condHelper.convertToCondition(colInd, fail));
        if (fail) return;
    }

    vector<map<Table *, shared_ptr<Record>>> result;
    set<Table *> processedTables;

    bool firstQuery = true;
    for (pair<string, Table *> curTable : tables) {
        Table *&table = curTable.second;
        processedTables.insert(table);

        vector<RecordCondition> curConditions;
        for (RecordCondition &condition : queryConditions) {
            if ((condition.lTable == NULL || processedTables.find(condition.lTable) != processedTables.end()) &&
                (condition.rTable == NULL || processedTables.find(condition.rTable) != processedTables.end()))
                curConditions.push_back(condition);
        }

        if (firstQuery) {
            firstQuery = false;
            vector<shared_ptr<Record>> records = table->queryWithCondition(curConditions);
            map<Table *, shared_ptr<Record>> curRecord;
            for (shared_ptr<Record> r : records) {
                curRecord[table] = r;
                result.push_back(curRecord);
            }
        } else {
            vector<map<Table *, shared_ptr<Record>>> _result;

            for (map<Table *, shared_ptr<Record>> prevRecord : result) {
                vector<shared_ptr<Record>> records = table->queryWithConditionAndPrevRecord(curConditions, prevRecord);
                for (shared_ptr<Record> r : records) {
                    prevRecord[table] = r;
                    _result.push_back(prevRecord);
                }
            }
            result = _result;
        }
    }

    // Output result
    printf("[");
    for (const string &col : resColNames) {
        printf("%s,", col.c_str());
    }
    printf("\b]\n-------------\n");
    for (map<Table *, shared_ptr<Record>> &record : result) {
        printf("(");
        for (RecordColRef ref : colRef) {
            record[ref.table]->reconstruct()[ref.col]->print();
            printf(",");
        }
        printf("\b)\n");
    }

    for (pair<string, Table *> p : tables)
        delete p.second;
}

void DBMS::update(string tableName, deque<pair<RecordCondTbCol, DataNode>> &setCols, deque<RecordCondHelper> &conditions) {
    ASSERT_CUR_DATABASE();

    Table *table;
    GET_AND_ASSERT_TABLE(table, tableName.c_str());
    // preprocess the conditions
    map<string, map<string, pair<Table *, int>>> colInd;
    for (int i = 0; i < table->colCnt; i++) {
        map<string, pair<Table *, int>> elem;
        elem[string(table->name)] = make_pair(table, i);
        colInd[string(table->colName[i])] = elem;
    }
    vector<RecordCondition> queryConditions;
    for (RecordCondHelper &condHelper : conditions) {
        bool fail = false;
        queryConditions.push_back(condHelper.convertToCondition(colInd, fail));
        if (fail) return;
    }

    map<int, shared_ptr<DataNode>> colRefSet;
    for (const pair<RecordCondTbCol, DataNode> &p : setCols) {
        RecordCondTbCol tbCol = p.first;
        bool fail = false;
        RecordColRef ref = tbCol.convertToColRef(colInd, fail);
        shared_ptr<DataNode> pData = make_shared<DataNode>(p.second);

        int i = ref.col;
        if (!(table->colType[i] == pData->type ||
                (((table->colSpecialType[i] & TableColInfo::NOTNULL) == 0) && pData->type == TYPE_COL_NULL))) {
            printf("** Error: value type not match with column type on column '%s'.\n", table->colName[i]);
            return;
        }

        colRefSet[ref.col] = pData;
        if (fail) return;
    }

    vector<shared_ptr<Record>> result = table->queryWithCondition(queryConditions);
    int udCnt = 0;
    for (shared_ptr<Record> rec : result) {
        udCnt++;
        rec->modify(colRefSet);
    }
    printf("* Success: %d records updated in table '%s'.\n", udCnt, table->name);
}

void DBMS::exit() {
    exitCurrent();
    puts("Database manage system quits normally.");
    quit = 1;
}

