#include "common/SharedMutex.h"
#include "common/Logging.h"

namespace sdb {

SharedMutex::SharedMutex() {
  if (0 != pthread_rwlock_init(&rwlock_, nullptr)) {
    LOG(FATAL) << "Fails to init a read write lock!";
  }
}

SharedMutex::~SharedMutex() {
  if (0 != pthread_rwlock_destroy(&rwlock_)) {
    LOG(FATAL) << "Fails to destroy a read write lock!";
  }
}

void SharedMutex::lock_shared() {
  if (0 != pthread_rwlock_rdlock(&rwlock_)) {
    LOG(FATAL) << "Fails to read lock a mutex!";
  }
}

void SharedMutex::unlock_shared() {
  if (0 != pthread_rwlock_unlock(&rwlock_)) {
    LOG(FATAL) << "Fails to unlock a mutex!";
  }
}

void SharedMutex::lock() {
  if (0 != pthread_rwlock_wrlock(&rwlock_)) {
    LOG(FATAL) << "Fails to write lock a mutex!";
  }
}

void SharedMutex::unlock() {
  if (0 != pthread_rwlock_unlock(&rwlock_)) {
    LOG(FATAL) << "Fails to unlock a mutex!";
  }
}

} // sdb
