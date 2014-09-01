#include "common/Range.h"
#include "common/Logging.h"
#include "common/UnitTest.h"

using namespace sdb;
using namespace std;


TEST(Range, testSimpleRange) {
  string s("hello");
  Range range(s);

  ASSERT_EQ(*range.begin(), 'h');
  ASSERT_EQ(*(range.end() - 1), 'o');
  ASSERT_EQ(range.size(), s.size());
}

TEST(Range, testRangeFor) {
  string s("hello");
  Range range(s);

  string tmp;
  for(auto ch : range) {
    tmp.push_back(ch);
  }

  ASSERT_EQ(s, tmp);
}

TEST(Range, testAccessor) {
  string s("hello");
  Range range(s);

  for (int i = 0; i < s.size(); ++i) {
    ASSERT_EQ(s[i], range[i]);
  }
}

TEST(Range, testEqual) {
  string s("hello");
  Range r1(s);
  Range r2(s);
  ASSERT_TRUE(r1 == r2);
}

TEST(Range, testLessThan) {
  string s1("world");
  string s2("hello");
  Range r1(s1);
  Range r2(s2);
  ASSERT_FALSE(r1 < r2);
  ASSERT_FALSE(r1 == r2);
}

TEST(Range, testFindSubString) {
  string s1("hello, world");
  string s2("world");

  Range main(s1);
  Range sub(s2);

  auto it = main.find(sub);

  ASSERT_GT(it, main.begin());
  ASSERT_LT(it, main.end());

  string s3("random");
  Range sub2(s3);

  it = main.find(sub2);

  ASSERT_EQ(it, main.end());
}

TEST(Range, testWriteReadIoRange) {
  vector<string> toBeWritten = {
    "hello",
    "world",
  };

  IoRange ioRange;

  for (auto&s : toBeWritten) {
    ioRange.append(s);
  }

  for (auto& s : toBeWritten) {
    auto range = ioRange.read(s.size());
    Range original(s);
    ASSERT_TRUE(range == original);
  }
}

TEST(Range, testRelocate) {
  vector<string> toBeWritten = {
    "hello",
    "world",
  };

  IoRange ioRange;

  for (auto&s : toBeWritten) {
    ioRange.append(s);
  }

  ioRange.read(toBeWritten[0].size());

  auto beforeRelocate = ioRange.begin();
  ioRange.forceRelocate();
  auto afterRelocate = ioRange.begin();

  ASSERT_GT(beforeRelocate, afterRelocate);

  auto remain = ioRange.getRange();
  ASSERT_TRUE(remain == Range(toBeWritten[1]));
}

TEST(Range, testResizeIoRange) {
  string small("hello");
  string big(256* 1024, 'a');

  IoRange ioRange;

  ioRange.append(small);
  ioRange.append(big);

  auto smallRange = ioRange.read(small.size());
  ASSERT_TRUE(smallRange == Range(small));

  auto bigRange = ioRange.read(big.size());
  ASSERT_TRUE(bigRange == Range(big));
}

