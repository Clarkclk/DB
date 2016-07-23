#ifndef DATABASE_H
#define DATABASE_H

class Database;

#include "table.h"
#include "fileio/FileManager.h"
#include "bufmanager/BufPageManager.h"
#include <vector>

using std::vector;

class Database {
    bool isEmptyPage(int pageID);
    void recyclePage(int pageID);
    void allocPage(int pageID);

public:
    char name[252];
    int tableCnt; // max 64 tables
    char tableName[64][96];
    int tablePage[64];
    unsigned char pageBit[512]; // max support 512 * 8 = 4096 pages.
    int curPage = 0;

    static const int DATABASE_STRUCT_LEN = /*256+64*96+64*4+512+4=*/7172;

    FileManager *fm;
    BufPageManager *bpm;
    int fileID;

    void flush();
    int getPage(bool doFlush = true);

    bool dropTable(const char *tableName);
    Table *createTable(const char *name, vector<TableColInfo> colInfo);
    Table *loadTable(const char *name);

    static Database *createDatabase(const char *filename, const char *name);
    static Database *loadDatabase(const char *filename);

    ~Database();
private:
    Database();
};

#endif // DATABASE_H
