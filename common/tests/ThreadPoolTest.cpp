#include "common/ThreadPool.h"
#include "common/Logging.h"
#include "common/UnitTest.h"

#include <map>
#include <string>

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

using namespace sdb;
using namespace std;


TEST(ThreadPool, testWithFuture) {
  FORK {
    ThreadPool tp(1);
    future<bool> fut = tp.async([]() -> bool { return true; });
    ASSERT_TRUE(fut.get());
    tp.drain();
  };
}

TEST(ThreadPool, testMoreTasks) {
  FORK {
    ThreadPool tp(1);
    vector<future<bool>> futs(10);

    for (int i = 0; i < 10; ++i) {
      futs[i] = tp.async([]() ->bool {
        usleep(10);
        return true;
      });
    }

    for (auto& f : futs) {
      ASSERT_TRUE(f.get());
    }

    tp.drain();
  };
}
