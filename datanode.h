#ifndef DATANODE_H
#define DATANODE_H

#include <memory>

using std::shared_ptr;

enum SQL_Operators {
    SQL_OP_EQ,
    SQL_OP_NE,
    SQL_OP_LT,
    SQL_OP_LE,
    SQL_OP_GT,
    SQL_OP_GE,
};

class DataNode
{
public:
    void *data;
    short type;
    unsigned short length;

    static bool compare(SQL_Operators op, shared_ptr<DataNode> pd1, shared_ptr<DataNode> pd2);
    static bool compare(SQL_Operators op, const DataNode &pd1, const DataNode &pd2);
    bool operator<(const DataNode &x) const;
    bool operator==(const DataNode &x) const;
    bool operator!=(const DataNode &x) const;

    void print() const;

    DataNode();
    DataNode(int data);
    DataNode(float data);
    DataNode(const char *data);
    DataNode(const char *data, int length);

    DataNode(const DataNode &dataNode);
    DataNode &operator=(const DataNode &dataNode);
    ~DataNode();
private:
    void recycle();
};

#endif // DATANODE_H
