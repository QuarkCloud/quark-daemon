
#ifndef QKRTL_SYNCH_H
#define QKRTL_SYNCH_H 1

#include "qkrtl/Compile.h"

namespace qkrtl {

class QKRTLAPI Mutex {
public:
    Mutex();
    virtual ~Mutex();

    virtual bool lock();
    virtual void unlock();
};

class QKRTLAPI Locker {
public:
    Locker(Mutex& mutex);
    virtual ~Locker();
    bool lock();
    void unlock();
private:
    Mutex& guard_;
};

class QKRTLAPI Waiter {
public:
    Waiter();
    virtual ~Waiter();

    bool wait();
    virtual bool waitFor(int timeout); //∫¡√Îº∂
    virtual void notifyOne();
    virtual void notifyAll();
    virtual int waiterCount() const;
};

}
#endif /**QKRTL_SYNCH_H*/
