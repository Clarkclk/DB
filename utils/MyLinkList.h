#ifndef MY_LINK_LIST
#define MY_LINK_LIST
//template <int LIST_NUM, int cap>
class MyLinkList {
private:
	struct ListNode {
		int next;
		int prev;
	};
	int cap;
	int LIST_NUM;
	ListNode* a;
	void link(int prev, int next);
public:
	void del(int index);
	void insert(int listID, int ele);
	void insertFirst(int listID, int ele);
	int getFirst(int listID);
	int next(int index);
	bool isHead(int index);
	bool isAlone(int index);
	MyLinkList(int c, int n);
};
#endif
