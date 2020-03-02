#include "LiteralList.h"

LiteralList::LiteralList(const int n)
    : len(0) {
    head = new LiteralListNode();
    tail = new LiteralListNode();
    head->next = tail;
    tail->prev = head;
    for (int i = 1; i <= n; ++i) {
        insertBack(i);
    }
}

LiteralList::~LiteralList() {
    LiteralListNode* p;
    while ((p = popFront()) != nullptr) {
        delete p;
    }
    delete head;
    delete tail;
}

void LiteralList::insertBack(const int n) {
    LiteralListNode* node = new LiteralListNode();
    node->is_decide = false;
    insertBack(node);
}

void LiteralList::insertBack(LiteralListNode* node) {
    node->next = tail;
    node->prev = tail->prev;
    tail->prev->next = node;
    tail->prev = node;
    ++len;
}

LiteralListNode* LiteralList::popFront() {
    if (len == 0) {
        return nullptr;
    }
    LiteralListNode* node = head->next;
    node->next->prev = head;
    head->next = node->next;
    node->next = nullptr;
    node->prev = nullptr;
    return node;
}
