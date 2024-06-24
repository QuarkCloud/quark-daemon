
#ifndef QKRTL_SEQUENCER_H
#define QKRTL_SEQUENCER_H 1

#include "qkrtl/Compile.h"
#include "qkrtl/Sequence.h"
#include "qkrtl/SystemInfo.h"

namespace qkrtl {

/**
    value�����б�ʹ�õ����к��У��������кš�next����ȡֵҪ���ڸ�ֵ
    gating������δ��ʹ�õ����к��У���С�����кš�
    ����������֮ǰ�����֮��gating = value + 1
    ���ڱ�����ķ�ΧΪ[gating , value]
*/
class QKRTLAPI Sequencer {
public:
    Sequencer();
    virtual ~Sequencer();
    /**
        ���fulfill==true����ʾҪ�������е�size��
        ���fulfill==false����ʾ����Ҫ����size���ж��پͷ��ض��١�
    */
    virtual int next(int64_t& sequence, int64_t barrier, int maxSize = 1, bool fulfill = true);
    virtual bool commit(int64_t sequence, int maxSize = 1);
    virtual int64_t getValue() const;
    virtual int64_t tryGetValue() const;
    virtual int64_t getGating() const;
    virtual int64_t tryGetGating() const;
};

class QKRTLAPI NotSafeSequencer : public Sequencer {
public:
    NotSafeSequencer();
    virtual ~NotSafeSequencer();

    virtual int next(int64_t& sequence, int64_t barrier, int maxSize = 1, bool fulfill = true);
    virtual bool commit(int64_t sequence, int maxSize = 1);
    virtual int64_t getValue() const;
    virtual int64_t tryGetValue() const;
    virtual int64_t getGating() const;
    virtual int64_t tryGetGating() const;

    static const int kPaddingSize = SystemInfo::kCacheLine - sizeof(int64_t);
private:
    char padding0[kPaddingSize] = {};
    int64_t value_;
    char padding1[kPaddingSize] = {};
    int64_t gating_;
    char padding2[kPaddingSize] = {};
};
 
class SafeSequencer : public Sequencer {
public:
    QKRTLAPI SafeSequencer();
    QKRTLAPI virtual ~SafeSequencer();

    QKRTLAPI virtual int next(int64_t& sequence, int64_t barrier, int maxSize = 1, bool fulfill = true);
    QKRTLAPI virtual bool commit(int64_t sequence, int maxSize = 1);

    QKRTLAPI virtual int64_t getValue() const;
    QKRTLAPI virtual int64_t tryGetValue() const;
    QKRTLAPI virtual int64_t getGating() const;
    QKRTLAPI virtual int64_t tryGetGating() const;

    inline Sequence& value() { return value_; }
    inline Sequence& gating() { return gating_; }
private:
    Sequence value_;
    Sequence gating_;   
};
}
#endif /**QKRTL_SEQUENCER_H*/
