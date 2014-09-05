#ifndef COMMON_SHAREDMUTEX_H
#define COMMON_SHAREDMUTEX_H

#include <pthread.h>

namespace sdb {

/// A generic shared lock template class
template <class mutex_class>
class shared_lock {
 public:

  explicit shared_lock(const mutex_class& mt)
    : mt_(const_cast<mutex_class*>(&mt)) {
    mt_->lock_shared();
  }

  shared_lock() : mt_(nullptr) {}

  shared_lock(shared_lock<mutex_class>&& l) : mt_(l.mt_) {
    l.mt_ = nullptr;
  }

  ~shared_lock() {
    if (mt_) {
      mt_->unlock_shared();
    }
  }

 private:

  mutex_class *mt_;

};


// c++11 does not provide a shared mutex yet, implement a wrapper
// of my own
class SharedMutex {
 public:

  SharedMutex();

  ~SharedMutex();

  // read-only lock
  void lock_shared();

  // unlock read-only lock
  void unlock_shared();

  // write lock
  void lock();

  // unlock write lock
  void unlock();

 private:

  pthread_rwlock_t rwlock_;
};

}

#endif // COMMON_SHAREDMUTEX_H
