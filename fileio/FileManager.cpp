#include "FileManager.h"

int FileManager::_createFile(const char* name) {
    FILE *f = fopen(name, "a+");
	if (f == NULL) {
		cout << "fail" << endl;
		return -1;
	}
	fclose(f);
	return 0;
}
int FileManager::_openFile(const char* name, int fileID) {
    FILE *f = fopen(name, "rb+");
    if (f == NULL) {
		return -1;
	}
	fd[fileID] = f;
	return 0;
}

/*
 * FilManager构造函数
 */
FileManager::FileManager() {
	fm = new MyBitMap(MAX_FILE_NUM, 1);
	tm = new MyBitMap(MAX_TYPE_NUM, 1);
}
/*
 * @函数名writePage
 * @参数fileID:文件id，用于区别已经打开的文件
 * @参数pageID:文件的页号
 * @参数buf:存储信息的缓存(4字节无符号整数数组)
 * @参数off:偏移量
 * 功能:将buf+off开始的2048个四字节整数(8kb信息)写入fileID和pageID指定的文件页中
 * 返回:成功操作返回0
 */
int FileManager::writePage(int fileID, int pageID, BufType buf, int off) {
    FILE *f = fd[fileID];
	off_t offset = pageID;
	offset = (offset << PAGE_SIZE_IDX);
    off_t error = fseek(f, offset, SEEK_SET);
    if (error != 0) {
		return -1;
	}
	BufType b = buf + off;
    error = fwrite((void*) b, 1, PAGE_SIZE, f);
    if (error != PAGE_SIZE) {
        return -1;
    }
	return 0;
}
/*
 * @函数名readPage
 * @参数fileID:文件id，用于区别已经打开的文件
 * @参数pageID:文件页号
 * @参数buf:存储信息的缓存(4字节无符号整数数组)
 * @参数off:偏移量
 * 功能:将fileID和pageID指定的文件页中2048个四字节整数(8kb)读入到buf+off开始的内存中
 * 返回:成功操作返回0
 */
int FileManager::readPage(int fileID, int pageID, BufType buf, int off) {
	//int f = fd[fID[type]];
    FILE *f = fd[fileID];
	off_t offset = pageID;
	offset = (offset << PAGE_SIZE_IDX);
    off_t error = fseek(f, offset, SEEK_SET);
    if (error != 0) {
		return -1;
	}
	BufType b = buf + off;
    error = fread((void*) b, 1, PAGE_SIZE, f);
    if (error != PAGE_SIZE) {
        return -1;
    }
	return 0;
}
/*
 * @函数名closeFile
 * @参数fileID:用于区别已经打开的文件
 * 功能:关闭文件
 * 返回:操作成功，返回0
 */
int FileManager::closeFile(int fileID) {
	fm->setBit(fileID, 1);
    FILE *f = fd[fileID];
    fclose(f);
	return 0;
}
/*
 * @函数名createFile
 * @参数name:文件名
 * 功能:新建name指定的文件名
 * 返回:操作成功，返回true
 */
bool FileManager::createFile(const char* name) {
	_createFile(name);
	return true;
}
/*
 * @函数名openFile
 * @参数name:文件名
 * @参数fileID:函数返回时，如果成功打开文件，那么为该文件分配一个id，记录在fileID中
 * 功能:打开文件
 * 返回:如果成功打开，在fileID中存储为该文件分配的id，返回true，否则返回false
 */
bool FileManager::openFile(const char* name, int& fileID) {
	fileID = fm->findLeftOne();
	fm->setBit(fileID, 0);
    if (_openFile(name, fileID) != 0) {
        fileID = -1;
        return false;
    }
	return true;
}
int FileManager::newType() {
	int t = tm->findLeftOne();
	tm->setBit(t, 0);
	return t;
}
void FileManager::closeType(int typeID) {
	tm->setBit(typeID, 1);
}
void FileManager::shutdown() {
	delete tm;
	delete fm;
}
int FileManager::deleteFile(const char *name) {
    return remove(name);
}

FileManager::~FileManager() {
	this->shutdown();
}
