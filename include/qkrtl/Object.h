
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
    int status;     //0��ʾ��ʼ״̬��1��ʾռ��״̬
    int next;      //�¸�����
} ObjectNode;

/**
    �౾����Ҫ��������Ҫ���ھ�����䡣
    �ڲ����ɾ��ʱ���������ⲿ��������ѯʱ����Ҫ��
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
        �����������Ҫ��ֱ���滻ԭ���Ķ�����ΪC++������C�ṹ����������ֱ���滻
        ����ָ�롣C++���麯�����׼̳�ʵ�֣�ȴ�����滻��
    */
    QKRTLAPI bool exchange(int oid, Object * obj , Object*& old);

    QKRTLAPI Object* find(int oid);
    QKRTLAPI const Object* find(int oid) const;
    QKRTLAPI bool valid(int handle) const;
    inline int size() const { return size_; }
    inline int maxSize() const { return maxSize_; }

private:
    SpinLock guard_;

    int lastIndex_; //˳�����
    int head_;      //������䣬ֻ�����ͷź󣬲Ż���ֵ
    int tail_;
    int size_;

    ObjectNode* nodes_;
    int maxSize_;
};
}

#endif /**QKRTL_OBJECT_H*/
