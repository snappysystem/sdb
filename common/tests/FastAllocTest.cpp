#include "common/FastAlloc.h"
#include "common/UnitTest.h"

#include <vector>
#include <map>
#include <chrono>

using namespace std;
using namespace sdb;
using namespace std::chrono;

// enable this if you want to print out perf info
const bool printPerf = false;


TEST(FastAlloc, testVector) {
  vector<int, FastAlloc<int>> vi;
  for (int i = 0; i < 2048; ++i) {
    vi.push_back(i);
  }
  for (int i = 2047; i >= 0; --i) {
    ASSERT_EQ(i, vi.back());
    vi.pop_back();
  }
}

TEST(FastAlloc, testMap) {
  map<int, int, less<int>, FastAlloc<pair<const int, int>>> m;
  for (int i = 0; i < 2048; ++i) {
    m.insert({i, i});
  }

  int i = 0;
  for (auto& p : m) {
    ASSERT_EQ(p.first, i);
    ASSERT_EQ(p.second, i);
    ++i;
  }

  // test updates
  m.erase(35);

  {
    auto it = m.lower_bound(35);
    ASSERT_TRUE(it != m.end());
    ASSERT_EQ(it->first, 36);
  }
}

TEST(FastAlloc, testMapDifferentAlloc) {
  map<int, int, less<int>, FastAlloc<char>> m;
  for (int i = 0; i < 2048; ++i) {
    m.insert({i, i});
  }

  int i = 0;
  for (auto& p : m) {
    ASSERT_EQ(p.first, i);
    ASSERT_EQ(p.second, i);
    ++i;
  }

  // test updates
  m.erase(35);

  {
    auto it = m.lower_bound(35);
    ASSERT_TRUE(it != m.end());
    ASSERT_EQ(it->first, 36);
  }
}

TEST(FastAlloc, testPerf) {
  {
    map<int, int, less<int>, FastAlloc<char, uint64_t, 256*1024>> m;
    auto beg = steady_clock::now();

    for (int i = 0; i < 4096; ++i) {
      m.insert({i, i});
    }

    auto end = steady_clock::now();
    auto ms = duration_cast<microseconds>(end - beg).count();

    if (printPerf) {
      cout << "fast alloc takes " << ms << " microseconds" << endl;
    }
  }

  {
    map<int, int> m;
    auto beg = steady_clock::now();

    for (int i = 0; i < 4096; ++i) {
      m.insert({i, i});
    }

    auto end = steady_clock::now();
    auto ms = duration_cast<microseconds>(end - beg).count();

    if (printPerf) {
      cout << "standard alloc takes " << ms << " microseconds" << endl;
    }
  }
}
