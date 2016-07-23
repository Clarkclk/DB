#include "datanode.h"

#include "table.h"
#include <cstdio>

bool DataNode::compare(SQL_Operators op, shared_ptr<DataNode> pd1, shared_ptr<DataNode> pd2) {
    DataNode &d1 = *pd1, &d2 = *pd2;
    return compare(op, d1, d2);
}

bool DataNode::compare(SQL_Operators op, const DataNode &d1, const DataNode &d2) {
    if (d1.type == TYPE_COL_NULL || d2.type == TYPE_COL_NULL) {
        switch (op) {
        case SQL_OP_EQ: return d1.type == d2.type;
        case SQL_OP_NE: return d1.type != d2.type;
        default: return false;
        }
    }
    if (d1.type != d2.type) {
        printf("Error: DataType not match when comparing: <");
        d1.print();
        printf(">, <");
        d2.print();
        printf(">\n");
        return false;
    }
    switch (d1.type) {
    case TYPE_COL_INT:
        switch (op) {
        case SQL_OP_EQ: return *(int *)d1.data == *(int *)d2.data;
        case SQL_OP_NE: return *(int *)d1.data != *(int *)d2.data;
        case SQL_OP_LT: return *(int *)d1.data <  *(int *)d2.data;
        case SQL_OP_LE: return *(int *)d1.data <= *(int *)d2.data;
        case SQL_OP_GT: return *(int *)d1.data >  *(int *)d2.data;
        case SQL_OP_GE: return *(int *)d1.data >= *(int *)d2.data;
        }
        break;
    case TYPE_COL_FLOAT:
        switch (op) {
        case SQL_OP_EQ: return *(float *)d1.data == *(float *)d2.data;
        case SQL_OP_NE: return *(float *)d1.data != *(float *)d2.data;
        case SQL_OP_LT: return *(float *)d1.data <  *(float *)d2.data;
        case SQL_OP_LE: return *(float *)d1.data <= *(float *)d2.data;
        case SQL_OP_GT: return *(float *)d1.data >  *(float *)d2.data;
        case SQL_OP_GE: return *(float *)d1.data >= *(float *)d2.data;
        }
        break;
    case TYPE_COL_VARCHAR:
        int len = d1.length < d2.length ? d1.length : d2.length;
        switch (op) {
        case SQL_OP_EQ: return strncmp((char *)d1.data, (char *)d2.data, len) == 0;
        case SQL_OP_NE: return strncmp((char *)d1.data, (char *)d2.data, len) != 0;
        case SQL_OP_LT: return strncmp((char *)d1.data, (char *)d2.data, len) <  0;
        case SQL_OP_LE: return strncmp((char *)d1.data, (char *)d2.data, len) <= 0;
        case SQL_OP_GT: return strncmp((char *)d1.data, (char *)d2.data, len) > 0;
        case SQL_OP_GE: return strncmp((char *)d1.data, (char *)d2.data, len) >= 0;
        }
        break;
    }

    printf("Error: Operator not supported by DataType.\n");
    return false;
}

void DataNode::print() const {
    switch (type) {
    case TYPE_COL_INT:
        printf("%d", *((const int *)data));
        break;
    case TYPE_COL_FLOAT:
        printf("%f", *((const float *)data));
        break;
    case TYPE_COL_VARCHAR:
        printf("'%s'", (const char *)data);
        break;
    case TYPE_COL_NULL:
        printf("NULL");
        break;
    }
}

bool DataNode::operator<(const DataNode &x) const {
    return compare(SQL_OP_LT, *this, x);
}

bool DataNode::operator==(const DataNode &x) const {
    return compare(SQL_OP_EQ, *this, x);
}

bool DataNode::operator!=(const DataNode &x) const {
    return compare(SQL_OP_NE, *this, x);
}

DataNode::DataNode() {
    type = TYPE_COL_NULL;
    length = 0;
    this->data = NULL;
}

DataNode::DataNode(int data) {
    type = TYPE_COL_INT;
    length = 4;
    this->data = new int(data);
}

DataNode::DataNode(float data) {
    type = TYPE_COL_FLOAT;
    length = sizeof(float);
    this->data = new float(data);
}

DataNode::DataNode(const char *data) {
    type = TYPE_COL_VARCHAR;
    this->length = strlen(data) + 1;
    this->data = new char[length];
    strncpy((char *)this->data, data, length);
    ((char *)this->data)[length - 1] = 0;
}

DataNode::DataNode(const char *data, int length) {
    type = TYPE_COL_VARCHAR;
    this->data = new char[length];
    this->length = length;
    strncpy((char *)this->data, data, length);
    ((char *)this->data)[length - 1] = 0;
}

DataNode::DataNode(const DataNode &dataNode) {
    type = dataNode.type;
    length = dataNode.length;
    data = new char[length];
    memcpy(data, dataNode.data, length);

    // printf("** Panic: unexpected copy constructor!\n");
}

void DataNode::recycle() {
    if (data) {
        switch(type) {
        case TYPE_COL_INT:
            delete (int *)data;
            break;
        case TYPE_COL_FLOAT:
            delete (float *)data;
            break;
        case TYPE_COL_VARCHAR:
            delete[] (char *)data;
            break;
        }
    }
}

DataNode &DataNode::operator=(const DataNode &dataNode) {
    type = dataNode.type;
    length = dataNode.length;

    if (data) {
        recycle();
        data = new char[length];
    }
    memcpy(data, dataNode.data, length);

    // printf("** Panic: unexpected assignment constructor!\n");
    return *this;
}

DataNode::~DataNode() {
    recycle();
}
