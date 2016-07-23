#include "record.h"

void Record::print() {
    printf("(");
    int i = 0;
    for (shared_ptr<DataNode> data : reconstruct()) {
        data->print();
        if (++i != table->colCnt) printf(", ");
    }
    printf(")\n");
}

void Record::remove() {
    int index;
    char *b = (char *)table->database->bpm->getPage(table->database->fileID, table->pages[pageN], index);
    b[recNo >> 3] &= ~(1 << (recNo & 0x07));
    table->recordCnt--;
    table->pageLeftSpace[pageN]--;
    table->flush();
    table->database->bpm->markDirty(index);
}

// Done: primary key duplication detection.
bool Record::modify(map<int, shared_ptr<DataNode>> modSet) {

    vector<shared_ptr<DataNode>> data = reconstruct();

    // duplication detection.
    for (pair<const int, set<DataNode>> &p : *table->primaryKey) {
        int ind = p.first;
        if (*modSet[ind] != *data[ind] && p.second.find(*modSet[ind]) != p.second.end()) { // duplicate primary key found.
            printf(" ** Error: Duplication detected on PRIMARY KEY constraint: '%s'.'%s', key = ", table->name, table->colName[ind]);
            modSet[ind]->print();
            puts(".");
            return false;
        }
    }

    for (pair<const int, shared_ptr<DataNode>> &p : modSet) {
        int col = p.first;
        shared_ptr<DataNode> pdata = p.second;
        char *b = this->recData;

        for (int i = 0; i < col; i++) {
            b += table->colLength[i] + ((table->colSpecialType[i] & TableColInfo::NOTNULL) == 0);
        }

        if ((table->colSpecialType[col] & TableColInfo::NOTNULL) == 0 && pdata->type == TYPE_COL_NULL)
            b[0] = 1;
        else if ((table->colSpecialType[col] & TableColInfo::NOTNULL) != 0 && pdata->type == TYPE_COL_NULL)
            printf("** Error: Cannot give NULL value to column #%d.\n", col);
        else
            memcpy(b + 1, pdata->data, table->colLength[col]);
    }

    // record the primary keys
    for (pair<const int, set<DataNode>> &p : *table->primaryKey) {
        int ind = p.first;
        p.second.erase(*data[ind]);
        p.second.insert(*modSet[ind]);
    }

    flush();
    return true;
}

void Record::flush() {
    int index;
    char *b = (char *)table->database->bpm->getPage(table->database->fileID, table->pages[pageN], index);
    memcpy(b + Table::TABLE_DATA_HEADER_LEN + table->recordLength * recNo, recData, table->recordLength);
    table->database->bpm->markDirty(index);
}

vector<shared_ptr<DataNode>> Record::reconstruct() {
    return reconstruct(table, recData);
}

vector<shared_ptr<DataNode>> Record::reconstruct(Table *table, const char *b) {
    vector<shared_ptr<DataNode>> record;
    if (!table) {
        printf("** Exception: refering NULL table from some record (reconstruct).\n");
        return record; // NULL table exception.
    }
    for (int i = 0; i < table->colCnt; i++) {
        if ((table->colSpecialType[i] & TableColInfo::NOTNULL) == 0) { // can be NULL
            if (b[0] == 1) { // NULL value
                record.push_back(make_shared<DataNode>());
                b += table->colLength[i] + 1;
                continue;
            } else {
                b += 1;
            }
        }
        switch (table->colType[i]) {
        case TYPE_COL_INT:
            record.push_back(make_shared<DataNode>(*(const int *)b));
            break;
        case TYPE_COL_FLOAT:
            record.push_back(make_shared<DataNode>(*(const float *)b));
            break;
        case TYPE_COL_VARCHAR:
            record.push_back(make_shared<DataNode>(b, table->colLength[i]));
            break;
        default:
            printf("** Error: Sorry, we cannot construct the data currently...");
        }
        b += table->colLength[i];
    }
    return record;
}

Record::Record() {
    table = NULL;
    pageN = 0;
    recNo = 0;
    recData = NULL;
    printf("** Exception: invalid record construction!\n");
}

Record::Record(Table *table, int pageN, int recNo) {
    this->table = table;
    this->pageN = pageN;
    this->recNo = recNo;
    recData = new char[table->recordLength];

    int index;
    char *b = (char *)table->database->bpm->getPage(table->database->fileID, table->pages[pageN], index);
    memcpy(recData, b + Table::TABLE_DATA_HEADER_LEN + recNo * table->recordLength, table->recordLength);
}

Record::Record(const Record &record) {
    table = record.table;
    pageN = record.pageN;
    recNo = record.recNo;
    recData = new char[table->recordLength];
    memcpy(recData, record.recData, table->recordLength);

    printf("** Panic: unexpected copy constructor!\n");
}

Record &Record::operator=(const Record &record) {
    if (!table || !record.table || table->recordLength != record.table->recordLength) {
        delete[] recData;
        recData = new char[record.table->recordLength];
    }

    table = record.table;
    pageN = record.pageN;
    recNo = record.recNo;

    memcpy(recData, record.recData, table->recordLength);

    printf("** Panic: unexpected assignment constructor!\n");
    return *this;
}

Record::~Record() {
    delete[] recData;
}

bool RecordCondition::processCondition(map<Table *, shared_ptr<Record>> records) {
    if (lTable) {
        lImmd = records[lTable]->reconstruct()[lCol];
    }
    if (rTable) {
        rImmd = records[rTable]->reconstruct()[rCol];
    }
    return DataNode::compare(op, lImmd, rImmd);
}

RecordCondition::RecordCondition(SQL_Operators op, Table *lTable, int lCol, Table *rTable, int rCol) {
    this->op = op;
    this->lTable = lTable;
    this->lCol = lCol;
    this->rTable = rTable;
    this->rCol = rCol;
}
RecordCondition::RecordCondition(SQL_Operators op, Table *lTable, int lCol, shared_ptr<DataNode> rImmd) {
    this->op = op;
    this->lTable = lTable;
    this->lCol = lCol;
    this->rTable = NULL;
    this->rImmd = rImmd;
}
RecordCondition::RecordCondition(SQL_Operators op, shared_ptr<DataNode> lImmd, Table *rTable, int rCol) {
    this->op = op;
    this->lTable = NULL;
    this->lImmd = lImmd;
    this->rTable = rTable;
    this->rCol = rCol;
}

static bool convertToTableCol(const string &col, const string &table, map<string, map<string, pair<Table *, int>>> &colInd, Table *&_Table, int &_Col) {
    auto result = colInd.find(col);
    if (result == colInd.end()) {
        printf("** Error: Cannot find column '%s' among the tables.\n", col.c_str());
        return false;
    }
    map<string, pair<Table *, int>> &colMap = result->second;
    if (table == "") {
        if (colMap.size() == 1) {
            _Table = colMap.begin()->second.first;
            _Col = colMap.begin()->second.second;
        } else {
            puts("");
            printf("** Error: Cannot determine column '%s' belongs to which of the following tables:\n", col.c_str());
            for (const pair<string, pair<Table *, int>> &p : colMap) {
                printf("'%s' ", p.first.c_str());
            }
            return false;
        }
    } else {
        auto result = colMap.find(table);
        if (result == colMap.end()) {
            printf("** Error: Cannot find column '%s' in table '%s'.\n", col.c_str(), table.c_str());
            return false;
        } else {
            _Table = result->second.first;
            _Col = result->second.second;
        }
    }
    return true;
}

RecordColRef::RecordColRef(Table *table, int col) {
    this->table = table;
    this->col = col;
}

RecordColRef RecordCondTbCol::convertToColRef(map<string, map<string, pair<Table *, int>>> &colInd, bool &fail) {
    Table *table;
    int col;

    if (!convertToTableCol(this->col, this->table, colInd, table, col)) {
        fail = true;
        return RecordColRef(NULL, 0);
    }
    return RecordColRef(table, col);
}

RecordCondTbCol::RecordCondTbCol(string table, string col) {
    this->table = table;
    this->col = col;
}

RecordCondTbCol::RecordCondTbCol(string col) {
    this->table = "";
    this->col = col;
}

RecordCondition RecordCondHelper::convertToCondition(map<string, map<string, pair<Table *, int>>> &colInd, bool &fail) {
    int type = 0;
    Table *_lTable, *_rTable;
    int _lCol, _rCol;

    RecordCondition falseCondition(SQL_OP_EQ, NULL, 0, NULL, 0);

    if (this->lCol != "") {
        type |= 2;
        if (!convertToTableCol(this->lCol, this->lTable, colInd, _lTable, _lCol)) {
            fail = true;
            return falseCondition;
        }
    }
    if (this->rCol != "") {
        type |= 1;
        if (!convertToTableCol(this->rCol, this->rTable, colInd, _rTable, _rCol)) {
            fail = true;
            return falseCondition;
        }
    }
    switch (type) {
    case 1: // 0b01
        if (lImmd->type != TYPE_COL_NULL && lImmd->type != _rTable->colType[_rCol]) {
            printf("** Error: Type not match on comparison between <");
            lImmd->print();
            printf("> and <%s.%s>.\n", _rTable->name, _rTable->colName[_rCol]);
            fail = true;
            return falseCondition;
        }
        return RecordCondition(op, lImmd, _rTable, _rCol);
    case 2: // 0b10
        if (rImmd->type != TYPE_COL_NULL && rImmd->type != _lTable->colType[_lCol]) {
            printf("** Error: Type not match on comparison between <");
            rImmd->print();
            printf("> and <%s.%s>.\n", _lTable->name, _lTable->colName[_lCol]);
            fail = true;
            return falseCondition;
        }
        return RecordCondition(op, _lTable, _lCol, rImmd);
    case 3: // 0b11
        if (_lTable->colType[_lCol] != _rTable->colType[_rCol]) {
            printf("** Error: Type not match on comparison between <%s.%s> ", _lTable->name, _lTable->colName[_lCol]);
            printf("and <%s.%s>.\n", _rTable->name, _rTable->colName[_rCol]);
            fail = true;
            return falseCondition;
        }
        return RecordCondition(op, _lTable, _lCol, _rTable, _rCol);
    default:
        puts("** Error: Unknown record type...");
        fail = true;
        return falseCondition;
    }
}

RecordCondHelper::RecordCondHelper(SQL_Operators op, RecordCondTbCol left, RecordCondTbCol right) {
    this->op = op;
    this->lTable = left.table;
    this->lCol = left.col;
    this->rTable = right.table;
    this->rCol = right.col;
}
RecordCondHelper::RecordCondHelper(SQL_Operators op, RecordCondTbCol left, DataNode rImmd) {
    this->op = op;
    this->lTable = left.table;
    this->lCol = left.col;
    this->rCol = "";
    this->rImmd = make_shared<DataNode>(rImmd);
}
RecordCondHelper::RecordCondHelper(SQL_Operators op, DataNode lImmd, RecordCondTbCol right) {
    this->op = op;
    this->lCol = "";
    this->lImmd = make_shared<DataNode>(lImmd);
    this->rTable = right.table;
    this->rCol = right.col;
}
