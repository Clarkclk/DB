#ifndef TABLE_H
#define TABLE_H

class Table;
struct TableColInfo;

enum DataTypeLabel {
    TYPE_COL_INT,
    TYPE_COL_FLOAT,
    TYPE_COL_VARCHAR,
    TYPE_COL_NULL,
};

#include "database.h"
#include "datanode.h"
#include "fileio/FileManager.h"
#include "bufmanager/BufPageManager.h"

#include "record.h"

#include <vector>
#include <map>
#include <set>
#include <memory>

using std::vector;
using std::pair;
using std::map;
using std::shared_ptr;

struct TableColInfo {
    short colType;
    unsigned short colLength;
    unsigned short specialType; // 0x01: NOT NULL; 0x02: PRIMARY KEY;
    char colName[64];

    static const unsigned short NOTNULL = 0x0001;
    static const unsigned short PRIMARYKEY = 0x0002;

    TableColInfo(short colType, unsigned short colLength, const char *colName, unsigned short specialType = 0x0000);
};

class Table {
    friend class Record;
    friend class DBMS;
public:
    char name[252];
    int colCnt; // max 64 columns
    short colType[64];
    unsigned short colLength[64];
    unsigned short colSpecialType[64];
    char colName[64][64];

    int recordCnt;
    int pageCnt; // max 128 pages

    int pages[128];
    int pageLeftSpace[128];

    static const int TABLE_STRUCT_LEN = /*256+64*2+64*2+64*2+64*64+4+4+128*4+128*4=*/5768;

    static const int TABLE_DATA_MAX_REC = 2048;
    static const int TABLE_DATA_HEADER_LEN = /*8192/4(bytes)/8(bits)=*/256;

    Database* database;
    int pageID;
    int recordLength, recPerPage;

    vector<shared_ptr<Record>> queryWithCondition(vector<RecordCondition> conditions);
    vector<shared_ptr<Record>> queryWithConditionAndPrevRecord(vector<RecordCondition> conditions, const map<Table *, shared_ptr<Record>> &prevRecord);
    void showData();

    bool insertRecord(vector<shared_ptr<DataNode>> record);

    void showInfo();
    void updateRecordLength();
    static Table *loadTable(Database *database, int pageID);
    static Table *createTable(Database *database, int pageID, const char *name, vector<TableColInfo> colInfo);

    ~Table();

private:
    vector<shared_ptr<Record>> queryTmp;
    vector<RecordCondition> queryCondTmp;
    map<Table *, shared_ptr<Record>> prevRecordTmp;

    map<int, set<DataNode>> *primaryKey = NULL;

    void incDataPage();
    void flush();
    void load();
    void setCol(vector<TableColInfo> colInfo);
    bool writeRecord(char *b, vector<shared_ptr<DataNode>> record);
    static void showDataAt(Table *table, shared_ptr<Record> record);
    static void queryWithConditionAt(Table *table, shared_ptr<Record> record);
    void traverseData(void (*callback)(Table *, shared_ptr<Record>));
    int findDataPos(char *b);

    static void buildPrimaryKeyAt(Table *table, shared_ptr<Record> record);
    void buildPrimaryKey();

    Table(Database *database, int pageID);
};

#endif // TABLE_H
