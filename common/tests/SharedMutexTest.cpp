#include "common/SharedMutex.h"
#include "common/ThreadPool.h"
#include "common/Logging.h"
#include "common/UnitTest.h"

using namespace sdb;
using namespace std;

TEST(SharedMutex, testReadLocks) {
  SharedMutex mt;

  mt.lock_shared();
  mt.lock_shared();
  mt.lock_shared();

  mt.unlock_shared();
  mt.unlock_shared();
  mt.unlock_shared();
}

TEST(SharedMutex, testSharedAndExclusiveLock) {
  SharedMutex mt;

  int val = 0;
  auto tp = new ThreadPool(2);

  mt.lock();

  for (int i = 0; i < 10; ++i) {
    tp->submit([&mt, &val]() {
      mt.lock_shared();
      ++val;
      mt.unlock_shared();
    });
  }

  ASSERT_EQ(val, 0);
  mt.unlock();

  tp->drain();
  delete tp;
}

TEST(SharedMutex, testExclusiveLock) {
  SharedMutex mt;

  int val = 0;
  auto tp = new ThreadPool(2);

  mt.lock();

  for (int i = 0; i < 10; ++i) {
    tp->submit([&mt, &val]() {
      mt.lock();
      ++val;
      mt.unlock();
    });
  }

  ASSERT_EQ(val, 0);
  mt.unlock();

  tp->drain();

  ASSERT_EQ(val, 10);
  delete tp;
}
