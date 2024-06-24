
#include "qkrtl/Sequencer.h"

namespace qkrtl {

Sequencer::Sequencer()
{
    //
}
Sequencer::~Sequencer()
{
    //
}
int Sequencer::next(int64_t& sequence, int64_t barrier, int maxSize, bool fulfill)
{
    return -1;
}
bool Sequencer::commit(int64_t sequence, int maxSize)
{
    return false;
}
int64_t Sequencer::getValue() const
{
    return 0;
}
int64_t Sequencer::tryGetValue() const
{
    return 0;
}
int64_t Sequencer::getGating() const
{
    return 0;
}
int64_t Sequencer::tryGetGating() const
{
    return 0;
}
NotSafeSequencer::NotSafeSequencer():value_(-1), gating_(0)
{

}
NotSafeSequencer::~NotSafeSequencer()
{
    //
}
int NotSafeSequencer::next(int64_t& sequence, int64_t barrier, int maxSize, bool fulfill)
{
    int64_t nextValue = value_ + 1;
    int avaibleSize = (int)(barrier - nextValue);
    if (fulfill == true)
    {
        if (avaibleSize < maxSize)
            return 0;
        else
            avaibleSize = maxSize;
    }
    else
    {
        if (avaibleSize > maxSize)
            avaibleSize = maxSize;
    }
    if (avaibleSize > 0)
    {
        sequence = nextValue;
        value_ = nextValue;
    }
    return avaibleSize;
}
bool NotSafeSequencer::commit(int64_t sequence, int maxSize)
{
    if (sequence != value_)
        return false;
    gating_ = sequence + maxSize;
    return true;
}
int64_t NotSafeSequencer::getValue() const
{
    return value_;
}
int64_t NotSafeSequencer::tryGetValue() const
{
    return value_;
}
int64_t NotSafeSequencer::getGating() const
{
    return gating_;
}
int64_t NotSafeSequencer::tryGetGating() const
{
    return gating_;
}

SafeSequencer::SafeSequencer():value_(-1) , gating_(0)
{
    //    
}
SafeSequencer::~SafeSequencer()
{
    //
}
int SafeSequencer::next(int64_t& sequence, int64_t barrier, int maxSize, bool fulfill)
{
    int avaibleSize = 0;
    bool success = false;
    while (success == false)
    {
        int64_t currentValue = value_.tryGetValue();
        avaibleSize = (int)(barrier - currentValue) - 1;
        if (fulfill == true)
        {
            if (avaibleSize < maxSize)
                break;
            else
                avaibleSize = maxSize;
        }
        else
        {
            if (avaibleSize > maxSize)
                avaibleSize = maxSize;
        }

        if (avaibleSize <= 0)
            break;

        int64_t nextValue = currentValue + avaibleSize;
        success = value_.compareAndSet(currentValue, nextValue);
        if (success == true)
        {
            sequence = currentValue + 1;
        }
    }
    return avaibleSize;
}
bool SafeSequencer::commit(int64_t sequence, int maxSize)
{
    int64_t nextValue = sequence + maxSize - 1;
    if (value_.tryGetValue() != nextValue)
        return false;
    
    return gating_.compareAndSet(sequence, sequence + maxSize);
}
int64_t SafeSequencer::getValue() const
{
    return value_.getValue();
}
int64_t SafeSequencer::tryGetValue() const
{
    return value_.tryGetValue();
}
int64_t SafeSequencer::getGating() const
{
    return gating_.getValue();
}
int64_t SafeSequencer::tryGetGating() const
{
    return gating_.tryGetValue();
}

}
