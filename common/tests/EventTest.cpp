#include "common/Event.h"
#include "common/ThreadPool.h"
#include "common/Logging.h"
#include "common/UnitTest.h"

#include <chrono>
#include <stdlib.h>

using namespace sdb;
using namespace std;
using namespace std::chrono;


int64_t nowInMs() {
  auto ts = steady_clock::now();
  return duration_cast<milliseconds>(ts.time_since_epoch()).count();
}

TEST(Event, testEventLoopAsync) {
  FORK {
    auto tp = new ThreadPool(1);
    auto eloop = new EventLoop(8);

    tp->submit([eloop]() { eloop->loop(); });

    auto fut = eloop->async([]() { return 32; });

    ASSERT_EQ(fut.get(), 32);

    eloop->quitLoopSoon();
    tp->drain();

    delete eloop;
    delete tp;
  };
}

TEST(Event, testSingleEventManagerAsync) {
  FORK {
    auto eventMgr = new EventManager(2, 1);
    auto fut = eventMgr->async([]() { return 32; });

    ASSERT_EQ(fut.get(), 32);

    eventMgr->drain();
    delete eventMgr;
  };
}

TEST(Event, testManyEventManagerAsync) {
  FORK {
    auto eventMgr = new EventManager(8, 1);
    vector<future<int>> futs;

    for (int i = 0; i < 100; ++i) {
      futs.push_back(eventMgr->async([i]() { return i; }));
    }

    for (int i = 0; i < 100; ++i) {
      ASSERT_EQ(futs[i].get(), i);
    }

    eventMgr->drain();
    delete eventMgr;
  };
}

TEST(Event, testSingleEventManagerSchedule) {
  FORK {
    auto eventMgr = new EventManager(2, 1);
    auto startTime = nowInMs();
    auto fut = eventMgr->schedule(startTime + 50, []() { return 32; });

    ASSERT_EQ(fut.get(), 32);

    auto endTime = nowInMs();
    ASSERT_LT(abs(endTime - startTime - 50), 2);

    eventMgr->drain();
    delete eventMgr;
  };
}

TEST(Event, testManyEventManagerSchedule) {
  FORK {
    auto eventMgr = new EventManager(8, 1);
    vector<future<int>> futs;
    auto startTime = nowInMs();

    for (int i = 0; i < 16; ++i) {
      futs.push_back(
        eventMgr->schedule(startTime + i * 20, [i]() {return i;}));
    }

    for (int i = 0; i < 16; ++i) {
      auto& f = futs[i];
      ASSERT_EQ(f.get(), i);
      auto ts = nowInMs();
      ASSERT_LT(abs(ts - startTime - i * 20), 2);
    }
      
    eventMgr->drain();
    delete eventMgr;
  };
}
