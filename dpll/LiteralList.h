#ifndef LITERAL_LIST_H
#define LITERAL_LIST_H

#include "common.h"

struct LiteralListNode {
    bool is_decide = false;
    LiteralListNode* next = nullptr;
    LiteralListNode* prev = nullptr;
};

class LiteralList {
public:
    LiteralList(const int n);
    ~LiteralList();
    void insertBack(const int n);
    void insertBack(LiteralListNode* node);
    LiteralListNode* popFront();

private:
    int len;
    LiteralListNode* head;
    LiteralListNode* tail;
};

#endif // !LITERAL_LIST_H
