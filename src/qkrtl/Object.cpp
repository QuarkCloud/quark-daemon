
#include "qkrtl/Object.h"
#include <string.h>
#include <stdlib.h>

namespace qkrtl {

Object::Object() : identifier_(kInvalidIdentifier)
{
    //
}
Object::~Object()
{
    //
}

ObjectManager::ObjectManager()
    :lastIndex_(0) , head_(0) ,  tail_(0) , size_(0) , 
    nodes_(NULL) , maxSize_(0)
{
    //
}
ObjectManager::~ObjectManager()
{
    final();
}

bool ObjectManager::init(int maxSize)
{
    int nodeSize = sizeof(ObjectNode) * maxSize;
    ObjectNode* nodes = (ObjectNode *)::malloc(nodeSize);
    if (nodes == NULL)
        return false;

    ::memset(nodes, 0, nodeSize);

    nodes_ = nodes;
    maxSize_ = maxSize;
    return true;
}
void ObjectManager::final()
{
    ObjectNode* nodes = NULL;
    {
        nodes = nodes_;
        nodes_ = NULL;
        maxSize_ = 0;
        head_ = tail_ = 0;
        lastIndex_ = 0;
    }
    
    if (nodes == NULL)
        return;
    ::free(nodes);
}
bool ObjectManager::insert(Object* obj)
{
    if (obj == NULL || valid(obj->identifier()) == true)
        return false;

    if (lastIndex_ >= 0 && (lastIndex_ + 1) < maxSize_)
    {
        //单向仍然存在有效索引
        int index = lastIndex_ + 1;
        ObjectNode& node = nodes_[index];
        if (node.status == 0 && node.next == 0)
        {
            ++lastIndex_;
            obj->identifier_ = index;
            node.object = obj;
            node.status = 1;
            node.next = 0;
            ++size_;
            return true;
        }
    }

    //从链表中分配
    if (head_ == 0 || tail_ == 0)
        return false;

    int index = head_;
    ObjectNode& node = nodes_[index];
    head_ = node.next;
    if (head_ == 0 || index == tail_)
        tail_ = 0;

    obj->identifier_ = index;
    node.object = obj;
    node.status = 1;
    node.next = 0;
    ++size_;
    return true;
}
bool ObjectManager::remove(Object* obj)
{
    if (obj == NULL)
        return false;

    int index = obj->identifier();
    if (index <= 0 || index >= maxSize_)
        return false;

    ObjectNode& node = nodes_[index];
    node.object = NULL;
    node.status = 0;
    node.next = 0;

    obj->identifier_ = Object::kInvalidIdentifier;
    if (head_ == 0 || tail_ == 0)
    {
        head_ = index;
        tail_ = index;
    }
    else
    {
        ObjectNode& tailNode = nodes_[tail_];
        tailNode.next = index;
        tail_ = index;
    }
    --size_;
    return true;
}
bool ObjectManager::remove(int oid, Object*& obj)
{
    obj = NULL;
    if (oid <= 0 || oid >= maxSize_)
        return false;

    ObjectNode& node = nodes_[oid];
    if (node.status == 0 || node.object == NULL)
        return false;

    obj = node.object;
    node.object = NULL;
    node.status = 0;
    node.next = 0;
    obj->identifier_ = Object::kInvalidIdentifier;

    if (head_ == 0 || tail_ == 0)
    {
        head_ = oid;
        tail_ = oid;
    }
    else
    {
        ObjectNode& tailNode = nodes_[tail_];
        tailNode.next = oid;
        tail_ = oid;
    }
    --size_;
    return true;
}
Object* ObjectManager::find(int oid)
{
    if (oid <= 0 || oid >= maxSize_)
        return NULL;

    ObjectNode& node = nodes_[oid];
    Object* object = node.object;
    if (node.status == 0 || object == NULL)
        return NULL;

    return object;    
}
const Object* ObjectManager::find(int oid) const
{
    if (oid <= 0 || oid >= maxSize_)
        return NULL;

    const ObjectNode& node = nodes_[oid];
    const Object* object = node.object;
    if (node.status == 0 || object == NULL)
        return NULL;

    return object;
}
bool ObjectManager::valid(int handle) const
{
    return (handle > 0 && handle < maxSize_); 
}


}
