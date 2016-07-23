#include "table.h"

TableColInfo::TableColInfo(short colType, unsigned short colLength, const char *colName, unsigned short specialType) {
    this->colType = colType;
    this->colLength = colLength;
    this->specialType = specialType;
    strncpy(this->colName, colName, 64);
}

void Table::queryWithConditionAt(Table *table, shared_ptr<Record> record) {
    table->prevRecordTmp[table] = record;
    for (RecordCondition &cond : table->queryCondTmp)
        if (!cond.processCondition(table->prevRecordTmp))
            return;

    table->queryTmp.push_back(record);
}

vector<shared_ptr<Record>> Table::queryWithCondition(vector<RecordCondition> conditions) {
    queryTmp.clear();
    prevRecordTmp.clear();
    queryCondTmp = conditions;
    traverseData(queryWithConditionAt);
    return queryTmp;
}

vector<shared_ptr<Record>> Table::queryWithConditionAndPrevRecord(vector<RecordCondition> conditions, const map<Table *, shared_ptr<Record>> &prevRecord) {
    queryTmp.clear();
    prevRecordTmp = prevRecord;
    queryCondTmp = conditions;
    traverseData(queryWithConditionAt);
    return queryTmp;
}

void Table::showInfo() {
    printf("Table '%s': (\n", name);
    for (int i = 0; i < colCnt; i++) {
        printf("\t%s ", colName[i]);
        switch (colType[i]) {
        case DataTypeLabel::TYPE_COL_INT:
            printf("INT"); break;
        case DataTypeLabel::TYPE_COL_VARCHAR:
            printf("VARCHAR(%d)", colLength[i]); break;
        case DataTypeLabel::TYPE_COL_FLOAT:
            printf("FLOAT"); break;
        default:
            printf("<Unknown type>");
        }
        if (colSpecialType[i] & TableColInfo::NOTNULL)
            printf(" NOT NULL");
        if (colSpecialType[i] & TableColInfo::PRIMARYKEY)
            printf(", PRIMARY KEY(%s)", colName[i]);
        printf(",\n");
    }
    printf(")\n#record = %d\n", this->recordCnt);
}

void Table::showDataAt(Table *, shared_ptr<Record> record) {
    // TODO: varchar data input check
    record->print();
}

void Table::traverseData(void (*callback)(Table *, shared_ptr<Record>)) {
    for (int pageN = 0; pageN < pageCnt; pageN++) {
        int page = pages[pageN], index;
        char *b = (char *)database->bpm->getPage(database->fileID, page, index);

        int rec = 0;
        for (int i = 0; i < TABLE_DATA_HEADER_LEN; i++) {
            unsigned char x = b[i];
            for (int j = 0; j < 8 && rec < recPerPage; j++, rec++) {
                if (x & 0x01) {
                    callback(this, make_shared<Record>(this, pageN, rec));
                }
                x >>= 1;
            }
            if (rec >= recPerPage)
                break;
        }
    }
}

void Table::showData() {
    showInfo();
    printf("------------\n");
    traverseData(showDataAt);
}

void Table::setCol(vector<TableColInfo> colInfo) {
    if (colCnt == 0) {
        colCnt = colInfo.size() < 64 ? colInfo.size() : 64;
        for (int i = 0; i < colCnt; i++) {
            colType[i] = colInfo[i].colType;
            colLength[i] = colInfo[i].colLength;
            colSpecialType[i] = colInfo[i].specialType;
            strncpy(colName[i], colInfo[i].colName, 64);
        }
        updateRecordLength();
    } else {
        printf("Error (setCol): Table %s is already initialized.\n", name);
    }
}

void Table::incDataPage() {
    int index, newPage = database->getPage();
    BufType b = database->bpm->getPage(database->fileID, newPage, index);
    memset(b, 0, TABLE_DATA_HEADER_LEN);
    database->bpm->markDirty(index);
    pages[pageCnt] = newPage;
    pageLeftSpace[pageCnt] = recPerPage;
    pageCnt++;
}

bool Table::writeRecord(char *b, vector<shared_ptr<DataNode>> record) {
    for (int i = 0; i < colCnt; i++) {
        if ((colSpecialType[i] & TableColInfo::NOTNULL) == 0) { // can be NULL
            if (record[i]->type == TYPE_COL_NULL) {
                b[0] = 1;
                b += colLength[i] + 1;
                continue;
            } else {
                b[0] = 0;
                b++;
            }
        } else { // cannot be null
            if (record[i]->type == TYPE_COL_NULL) {
                printf("** Error (writeRecord): cannot insert NULL value to NOT NULL column '%s'.\n", colName[i]);
                return false;
            }
        }
        memcpy(b, record[i]->data, colLength[i]);
        b += colLength[i];
    }
    return true;
}

int Table::findDataPos(char *b) {
    int rec = 0;
    for (int i = 0; i < TABLE_DATA_HEADER_LEN; i++) {
        unsigned char x = b[i];
        for (int j = 0; j < 8 && rec < recPerPage; j++, rec++) {
            if ((x & 0x01) == 0) {
                b[i] |= 1 << j;
                return rec;
            }
            x >>= 1;
        }
        if (rec >= recPerPage)
            break;
    }
    puts("Error(findDataPos): Data page has no more space.");
    return -1;
}

// Done: primary key duplication detection.
bool Table::insertRecord(vector<shared_ptr<DataNode>> record) {
    if (record.size() == colCnt) {
        int page;
        for (page = 0; page < pageCnt; page++)
            if (pageLeftSpace[page] != 0)
                break;
        if (page == pageCnt) {
            incDataPage();
        }
        recordCnt++;
        pageLeftSpace[page]--;

        // duplication detection
        buildPrimaryKey();
        for (pair<const int, set<DataNode>> &p : *primaryKey) {
            int ind = p.first;
            if (p.second.find(*record[ind]) != p.second.end()) { // duplicate primary key.
                printf(" ** Error: Duplication detected on PRIMARY KEY constraint: '%s'.'%s', key = ", name, colName[ind]);
                record[ind]->print();
                puts(".");
                return false;
            }
        }

        int index, pos;
        char *b = (char *)database->bpm->getPage(database->fileID, pages[page], index);
        pos = findDataPos(b);

        if (!writeRecord(b + TABLE_DATA_HEADER_LEN + pos * recordLength, record)) { // need roll back >_<
            recordCnt--;
            pageLeftSpace[page]++;
            b[pos >> 3] &= ~(1 << (pos & 0x07));
            return false;
        }

        // record the primary keys
        for (pair<const int, set<DataNode>> &p : *primaryKey) {
            int ind = p.first;
            p.second.insert(*record[ind]);
        }

        database->bpm->markDirty(index);

        flush();
    } else {
        printf("Error (insertRecord): record size(%d) incompatiable with column count(%d).\n", record.size(), colCnt);
        return false;
    }
    return true;
}

void Table::buildPrimaryKeyAt(Table *table, shared_ptr<Record> record) {
    vector<shared_ptr<DataNode>> data = record->reconstruct();
    for (pair<const int, set<DataNode>> &p : *table->primaryKey) {
        int ind = p.first;
        p.second.insert(*data[ind]);
    }
}

void Table::buildPrimaryKey() {
    if (!primaryKey) {
        primaryKey = new map<int, set<DataNode>>();
        for (int i = 0; i < colCnt; i++)
            if (colSpecialType[i] & TableColInfo::PRIMARYKEY)
                (*primaryKey)[i] = set<DataNode>();
        traverseData(buildPrimaryKeyAt);
    }
}

void Table::updateRecordLength() {
    recordLength = 0;
    for (int i = 0; i < colCnt; i++)
        recordLength += colLength[i] + ((colSpecialType[i] & TableColInfo::NOTNULL) == 0); // if !NOTNULL, we add a byte to indicate if the value is NULL
    if (recordLength == 0) {
        printf("Warning (updateRecordLength): no record in the table(%s)\n", name);
        return;
    }
    recPerPage = (8192 - TABLE_DATA_HEADER_LEN) / recordLength;
    if (recPerPage > TABLE_DATA_MAX_REC)
        recPerPage = TABLE_DATA_MAX_REC;
}

void Table::flush() {
    int index;

    BufType b = database->bpm->getPage(database->fileID, pageID, index);
    memcpy(b, this, TABLE_STRUCT_LEN);
    database->bpm->markDirty(index);
}

void Table::load() {
    int index;

    BufType b = database->bpm->getPage(database->fileID, pageID, index);
    memcpy(this, b, TABLE_STRUCT_LEN);

    updateRecordLength();
}

Table *Table::loadTable(Database *database, int pageID) {
    Table *table = new Table(database, pageID);
    table->load();
    return table;
}

Table *Table::createTable(Database *database, int pageID, const char *name, vector<TableColInfo> colInfo) {
    Table *table = new Table(database, pageID);
    strcpy(table->name, name);
    table->colCnt = 0; table->recordCnt = 0; table->pageCnt = 0;
    table->setCol(colInfo);
    table->flush();
    return table;
}

Table::Table(Database *database, int pageID) {
    this->database = database;
    this->pageID = pageID;
}
Table::~Table() {
    // TODO: close file
}
