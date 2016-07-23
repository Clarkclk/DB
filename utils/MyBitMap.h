#ifndef MY_BIT_MAP
#define MY_BIT_MAP
typedef unsigned int uint;
/*
#define LEAF_BIT 32
#define MAX_LEVEL 5
#define MAX_INNER_NUM 67
#define MOD 61
#define BIAS 5*/
#include <iostream>
using namespace std;

#define LEAF_BIT 32
#define MAX_LEVEL 5
#define MAX_INNER_NUM 67
//#define MOD 61
#define BIAS 5
extern unsigned char h[61];


class MyBitMap {
protected:
//	static const int LEAF_BIT = 32;
//	static const int MAX_LEVEL = 5;
//	static const int MAX_INNER_NUM = 10;
//	static const int MOD = 61;
//	static unsigned char h[MOD];
	static uint getMask(int k);
	uint* data;
	int size;
	int rootBit;
	int rootLevel;
	int rootIndex;
	uint inner[MAX_INNER_NUM];
	uint innerMask;
	uint rootMask;
	//virtual
	uint getLeafData(int index);
	//virtual
	void setLeafData(int index, uint v);
	int setLeafBit(int index, uint k);
	uint childWord(int start, int bitNum, int i, int j);
	void init();
	int _setBit(uint* start, int index, uint k);
	void updateInner(int level, int offset, int index, int levelCap, uint k);
	int _findLeftOne(int level, int offset, int pos, int prevLevelCap);
public:
//	static const int BIAS;/* = 5;*/
//	static void initConst();
	/* {
		for (int i = 0; i < 32; ++ i) {
			unsigned int k = (1 << i);
			MyBitMap::h[MyBitMap::_hash(k)] = i;
		}
	}
	*/
	static int _hash(uint i);
	static void initConst();
	static int getIndex(uint k);
	static uint lowbit(uint k);
	static void getPos(int index, int& pos, int& bit);
	uint data0();
	void setBit(int index, uint k);
	int findLeftOne();
	MyBitMap(int cap, uint k);
	MyBitMap(int cap, uint* da);
	void reLoad(uint* da);
};
#endif
