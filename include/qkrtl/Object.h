
#ifndef QKRTL_OBJECT_H
#define QKRTL_OBJECT_H 1

#include <string.h>
#include <stdint.h>
#include <atomic>
#include "qkrtl/Compile.h"
#include "qkrtl/SpinLock.h"

namespace qkrtl {
 
class QKRTLAPI Object {
public:
    Object();
    virtual ~Object();
    inline int identifier() const { return identifier_; }
    static const int kInvalidIdentifier = -1;
protected:
    friend class ObjectManager;
    int identifier_;
};

typedef struct ObjectNode {
    Object* object;
    int status;     //0表示初始状态，1表示占用状态
    int next;      //下个索引
} ObjectNode;

/**
    类本身不需要加锁，主要用于句柄分配。
    在插入和删除时，可以在外部加锁，查询时不需要。
*/
class ObjectManager {
public:
    QKRTLAPI ObjectManager();
    QKRTLAPI virtual ~ObjectManager();

    QKRTLAPI bool init(int maxSize);
    QKRTLAPI void final();

    QKRTLAPI bool insert(Object* obj);
    QKRTLAPI bool remove(Object* obj);
    QKRTLAPI bool remove(int oid , Object*& obj);

    /**
        这个函数很重要，直接替换原来的对象。因为C++对象不像C结构那样，可以直接替换
        函数指针。C++的虚函数容易继承实现，却不能替换。
    */
    QKRTLAPI bool exchange(int oid, Object * obj , Object*& old);

    QKRTLAPI Object* find(int oid);
    QKRTLAPI const Object* find(int oid) const;
    QKRTLAPI bool valid(int handle) const;
    inline int size() const { return size_; }
    inline int maxSize() const { return maxSize_; }

private:
    SpinLock guard_;

    int lastIndex_; //顺序分配
    int head_;      //链表分配，只有在释放后，才会有值
    int tail_;
    int size_;

    ObjectNode* nodes_;
    int maxSize_;
};
}

#endif /**QKRTL_OBJECT_H*/
