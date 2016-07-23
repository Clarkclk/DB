#include "database.h"

bool Database::isEmptyPage(int pageID) { // empty page = 0, alloced page = 1;
    return ~(pageBit[pageID >> 3] & (1 << (pageID & 0x07)));
}

void Database::recyclePage(int pageID) {
    pageBit[pageID >> 3] &= ~(1 << (pageID & 0x07));
}

void Database::allocPage(int pageID) {
    pageBit[pageID >> 3] |= (1 << (pageID & 0x07));
}

void Database::flush() {
    int index;
    BufType b = bpm->getPage(fileID, 0, index);
    memcpy(b, this, DATABASE_STRUCT_LEN);
    bpm->markDirty(index);
}

int Database::getPage(bool doFlush) {
    while (!isEmptyPage(curPage)) curPage++;
    int retPage = curPage++;
    allocPage(retPage);
    if (doFlush)
        flush();
    return retPage;
}

bool Database::dropTable(const char *tableName) {
    int oldPage = 0;
    for (int i = 0; i < tableCnt; i++) {
        if (oldPage) {
            strcpy(this->tableName[i - 1], this->tableName[i]);
            tablePage[i - 1] = tablePage[i];
        } else if (strcmp(this->tableName[i], tableName) == 0) {
            oldPage = tablePage[i];
        }
    }
    if (oldPage) {
        tableCnt--;
        Table *table = Table::loadTable(this, oldPage);
        for (int i = 0; i < table->pageCnt; i++)
            recyclePage(table->pages[i]);
        delete table;
        curPage = oldPage;
        flush();
        return true;
    } else
        return false;
}

Table *Database::createTable(const char *name, vector<TableColInfo> colInfo) {
    for (int i = 0; i < tableCnt; i++)
        if (strcmp(name, tableName[i]) == 0) {
            return NULL;
        }
    Table *table = Table::createTable(this, (tablePage[tableCnt] = getPage(false)), name, colInfo);
    strncpy(tableName[tableCnt], name, 96);
    tableCnt++;
    flush();
    return table;
}

Table *Database::loadTable(const char *name) {
    for (int i = 0; i < tableCnt; i++)
        if (strcmp(name, tableName[i]) == 0) {
            return Table::loadTable(this, tablePage[i]);
        }
    return NULL;
}

Database *Database::createDatabase(const char *filename, const char *name) {
    Database *database = new Database();

    strcpy(database->name, name);
    database->tableCnt = 0;
    memset(database->pageBit, 0, sizeof(database->pageBit));
    database->allocPage(0);
    database->curPage = 1;

    database->bpm->fileManager->createFile(filename);
    database->bpm->fileManager->openFile(filename, database->fileID);
    database->flush();

    return database;
}

Database *Database::loadDatabase(const char *filename) {
    int index;
    Database *database = new Database();
    if (database->bpm->fileManager->openFile(filename, database->fileID) == false) {
        delete database;
        return NULL;
    }
    BufType b = database->bpm->getPage(database->fileID, 0, index);
    memcpy(database, b, DATABASE_STRUCT_LEN);

    return database;
}

Database::~Database() {
    bpm->close();
    if (fileID != -1)
        bpm->fileManager->closeFile(fileID);
    delete fm;
    delete bpm;
}

Database::Database() {
    fm = new FileManager();
    bpm = new BufPageManager(fm);
    fileID = -1;
}
