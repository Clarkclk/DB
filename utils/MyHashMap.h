#ifndef MY_HASH_MAP
#define MY_HASH_MAP
#include "pagedef.h"
#include "MyLinkList.h"
/*
 * hash表的键
 */
struct DataNode_ {
	/*
	 * 第一个键
	 */
	int key1;
	/*
	 * 第二个键
	 */
	int key2;
};
/*
 * 两个键的hash表
 * hash表的value是自然数，在缓存管理器中，hash表的value用来表示缓存页面数组的下标
 */
class MyHashMap {
private:
	static const int A = 1;
	static const int B = 1;
	int CAP_, MOD_;
	MyLinkList* list;
    DataNode_* a;
	/*
	 * hash函数
	 */
	int hash(int k1, int k2);
public:
	/*
	 * @函数名findIndex
	 * @参数k1:第一个键
	 * @参数k2:第二个键
	 * 返回:根据k1和k2，找到hash表中对应的value
	 *           这里的value是自然数，如果没有找到，则返回-1
	 */
	int findIndex(int k1, int k2);
	/*
	 * @函数名replace
	 * @参数index:指定的value
	 * @参数k1:指定的第一个key
	 * @参数k2:指定的第二个key
	 * 功能:在hash表中，将指定value对应的两个key设置为k1和k2
	 */
	void replace(int index, int k1, int k2);
	/*
	 * @函数名remove
	 * @参数index:指定的value
	 * 功能:在hash表中，将指定的value删掉
	 */
	void remove(int index);
	/*
	 * @函数名getKeys
	 * @参数index:指定的value
	 * @参数k1:存储指定value对应的第一个key
	 * @参数k2:存储指定value对应的第二个key
	 */
	void getKeys(int index, int& k1, int& k2);
	/*
	 * 构造函数
	 * @参数c:hash表的容量上限
	 * @参数m:hash函数的mod
	 */
	MyHashMap(int c, int m);
};
#endif
