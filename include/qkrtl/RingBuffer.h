
#ifndef QKRTL_RING_BUFFER_H
#define QKRTL_RING_BUFFER_H 1

#include "qkrtl/Compile.h"
#include "qkrtl/PowerTwo.h"
#include "qkrtl/SystemInfo.h"
#include "qkrtl/Sequencer.h"

namespace qkrtl {

/**
    非线程安全，需要特别注意。
    这里之所以没有head和tail，因为保证索引的操作，需要分为带锁和无锁两种情况。
    带锁的索引，还需要区分多线程和单线程。
*/
template<typename DataType>
class RingBuffer {
public:
    RingBuffer()
        :datas_(NULL) , dataSize_(0) , indexShift_(0) , indexMask_(1)
    {
        //
    }
    virtual ~RingBuffer()
    {
        final();
    }
    bool init(int maxSize)
    {
        int shift = LogOfTwo(maxSize);
        int dataSize = 1 << shift;
        DataType* datas = new DataType[dataSize];
        if (datas == NULL)
            return false;
        datas_ = datas;
        dataSize_ = dataSize;
        indexShift_ = shift;
        indexMask_ = dataSize - 1;
        return true;
    }
    void final()
    {
        DataType* datas = datas_;
        if (datas_ == NULL)
            return;
        indexMask_ = 0;
        indexShift_ = 0;
        dataSize_ = 0;
        datas_ = NULL;
        if (datas != NULL)
            delete[] datas_;        
    }

    inline bool getAvaible(DataType*& start, int& size, int from, int maxSize = -1)
    {
        int avaibleSize = dataSize_ - from;
        if (maxSize >= 0 && maxSize < avaibleSize)
            avaibleSize = maxSize;

        start = getAddr(from);
        size = avaibleSize;

        return (start != NULL && size > 0);
    }

    inline bool getAvaible(DataType*& start, int& size, int64_t sequence, int maxSize = -1)
    {
        int from = calcIndex(sequence);
        return getAvaible(start, size, from, maxSize);
    }

    inline bool valid(int index) const { return (index >= 0 && index < dataSize_); }

    //可能会溢出，需要特别注意
    inline DataType& get(int index) { return datas_[index]; }
    inline const DataType& get(int index) const { return datas_[index]; }
    inline DataType * getAddr(int index) { return datas_ + index; }
    inline const DataType * getAddr(int index) const { return datas_ + index; }
    inline DataType& operator[](int index) { return datas_[index]; }
    inline const DataType& operator[](int index) const { return datas_[index]; }

    inline int dataSize() const { return dataSize_; }
    inline int indexShift() const { return indexShift_; }
    inline int indexMask() const { return indexMask_; }

    inline int calcIndex(int64_t sequence) const
    {
        return (((int)((sequence << 32) >> 32)) & indexMask_);
    }

private:
    DataType* datas_;
    int dataSize_;
    int indexShift_;
    int indexMask_;
};

/**
    RingIndex用于处理RingBuffer，非线程安全。
    而且只能处理单producer和单consumer，否则next会有冲突。
    之所以预先定义类，主要为了应用于简单场景。
*/
class QKRTLAPI RingIndex {
public:
    RingIndex();
    virtual ~RingIndex();

    int nextHead(int64_t& sequence , int size = 1 , bool fulfill = true);
    bool commitHead(int64_t sequence, int maxSize = 1);
    int nextTail(int64_t& sequence, int size = 1, bool fulfill = true);
    bool commitTail(int64_t sequence, int maxSize = 1);

    int backlog() const;

    void ringSize(int size);
    inline int ringSize() const { return ringSize_; }

    virtual Sequencer& getHead() = 0;
    virtual const Sequencer& getHead() const = 0;
    virtual Sequencer& getTail() = 0;
    virtual const Sequencer& getTail() const = 0;
private:
    int ringSize_;
};

class QKRTLAPI NotSafeRingIndex : public RingIndex {
public:
    NotSafeRingIndex();
    virtual ~NotSafeRingIndex();

    inline virtual Sequencer& getHead() { return head_; }
    inline virtual const Sequencer& getHead() const { return head_; }
    inline virtual Sequencer& getTail() { return tail_; }
    inline virtual const Sequencer& getTail() const { return tail_; }
private:
    NotSafeSequencer head_;
    NotSafeSequencer tail_;
};

class SafeRingIndex : public RingIndex {
public:
    QKRTLAPI SafeRingIndex();
    QKRTLAPI virtual ~SafeRingIndex();

    inline virtual Sequencer& getHead() { return head_; }
    inline virtual const Sequencer& getHead() const { return head_; }
    inline virtual Sequencer& getTail() { return tail_; }
    inline virtual const Sequencer& getTail() const { return tail_; }
private:
    SafeSequencer head_;
    SafeSequencer tail_;
};

/**
    只支持单写单读
*/
class SafeRingBuffer {
public:
    QKRTLAPI SafeRingBuffer();
    QKRTLAPI virtual ~SafeRingBuffer();

    QKRTLAPI bool init(int maxSize);
    QKRTLAPI void final();

    QKRTLAPI int read(char* buffer, int size);
    QKRTLAPI int write(const char* buffer, int size);

    inline int backlog() const { return index_.backlog(); }
private:
    RingBuffer<char> buffer_;
    SafeRingIndex index_;
};

}
#endif /**QKRTL_RING_BUFFER_H*/
