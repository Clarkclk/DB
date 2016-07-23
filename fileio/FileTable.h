#ifndef FILE_TABLE
#define FILE_TABLE
#include <string>
#include "../utils/MyBitMap.h"
#include <map>
#include <vector>
#include <set>
#include <fstream>
using namespace std;
class FileTable {
private:
	multiset<string> isExist;
	multiset<string> isOpen;
	vector<string> fname;
	vector<string> format;
	map<string, int> nameToID;
	string* idToName;
	MyBitMap* ft, *ff;
	int n;
	void load();
	void save();
public:
	int newTypeID();
	int newFileID(const string& name);
	bool ifexist(const string& name);
	void addFile(const string& name, const string& fm);
	int getFileID(const string& name);
	void freeTypeID(int typeID);
	void freeFileID(int fileID);
	string getFormat(string name);
	FileTable(int fn, int tn);
	~FileTable();
};
#endif
