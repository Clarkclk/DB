#ifndef RECORD_H
#define RECORD_H

struct Record;
struct RecordCondition;

#include "table.h"
#include "datanode.h"

#include <vector>
#include <map>
#include <memory>

using std::vector;
using std::map;
using std::shared_ptr;

struct Record {
    Table *table;
    int pageN, recNo;

    char *recData;

    void print();

    void remove();
    bool modify(map<int, shared_ptr<DataNode>> modSet);
    void flush();
    vector<shared_ptr<DataNode>> reconstruct();
    static vector<shared_ptr<DataNode>> reconstruct(Table *table, const char *b);

    Record();
    Record(const Record &record);
    Record &operator=(const Record &record);
    Record(Table *table, int pageN, int recNo);
    ~Record();
};

struct RecordCondition {
    Table *lTable, *rTable;
    int lCol, rCol;
    SQL_Operators op;
    shared_ptr<DataNode> lImmd, rImmd;

    bool processCondition(map<Table *, shared_ptr<Record>>);

    RecordCondition(SQL_Operators op, Table *lTable, int lCol, Table *rTable, int rCol);
    RecordCondition(SQL_Operators op, Table *lTable, int lCol, shared_ptr<DataNode> rImmd);
    RecordCondition(SQL_Operators op, shared_ptr<DataNode> lImmd, Table *rTable, int rCol);
};

struct RecordColRef {
    Table *table;
    int col;

    RecordColRef(Table *table, int col);
};

struct RecordCondTbCol {
    string table, col;

    RecordColRef convertToColRef(map<string, map<string, pair<Table *, int>>> &colInd, bool &fail);

    RecordCondTbCol(string table, string col);
    RecordCondTbCol(string col);
};

struct RecordCondHelper {
    string lTable, rTable;
    string lCol, rCol;
    SQL_Operators op;
    shared_ptr<DataNode> lImmd, rImmd;

    RecordCondition convertToCondition(map<string, map<string, pair<Table *, int>>> &colInd, bool &fail);

    RecordCondHelper(SQL_Operators op, RecordCondTbCol left, RecordCondTbCol right);
    RecordCondHelper(SQL_Operators op, RecordCondTbCol left, DataNode rImmd);
    RecordCondHelper(SQL_Operators op, DataNode lImmd, RecordCondTbCol right);
};

#endif // RECORD_H
