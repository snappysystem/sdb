#include "common/Serializer.h"
#include "common/Range.h"
#include "common/Logging.h"
#include "common/UnitTest.h"

using namespace std;
using namespace sdb;


TEST(Serializer, testProcessChar) {
  char val = 'a';
  char ret = '\0';

  {
    IoRange ioRange;
    Serializer<char>().append(ioRange, val);

    auto range = ioRange.getRange();

    ASSERT_TRUE(Deserializer<char>().parse(range, ret));
    ASSERT_EQ(val, ret);
    ASSERT_EQ(range.size(), 0);
  }

  ASSERT_EQ(Serializer<char>().sizeOf(val), 1);

  {
    IoRange ioRange;
    Serializer<char>().append(ioRange, val);
    auto range = ioRange.getRange();

    ASSERT_TRUE(Deserializer<char>().skip(range));
    ASSERT_EQ(range.size(), 0);
  }
}

TEST(Serializer, testProcessInt16) {
  int16_t val = 1616;
  int16_t ret = 0;

  {
    IoRange ioRange;
    Serializer<int16_t>().append(ioRange, val);

    auto range = ioRange.getRange();

    ASSERT_TRUE(Deserializer<int16_t>().parse(range, ret));
    ASSERT_EQ(val, ret);
    ASSERT_EQ(range.size(), 0);
  }

  ASSERT_EQ(Serializer<int16_t>().sizeOf(val), 2);

  {
    IoRange ioRange;
    Serializer<int16_t>().append(ioRange, val);
    auto range = ioRange.getRange();

    ASSERT_TRUE(Deserializer<int16_t>().skip(range));
    ASSERT_EQ(range.size(), 0);
  }
}

TEST(Serializer, testProcessInt32) {
  int32_t val = 16163232;
  int32_t ret = 0;

  {
    IoRange ioRange;
    Serializer<int32_t>().append(ioRange, val);

    auto range = ioRange.getRange();

    ASSERT_TRUE(Deserializer<int32_t>().parse(range, ret));
    ASSERT_EQ(val, ret);
    ASSERT_EQ(range.size(), 0);
  }

  ASSERT_EQ(Serializer<int32_t>().sizeOf(val), 4);

  {
    IoRange ioRange;
    Serializer<int32_t>().append(ioRange, val);
    auto range = ioRange.getRange();

    ASSERT_TRUE(Deserializer<int32_t>().skip(range));
    ASSERT_EQ(range.size(), 0);
  }
}

TEST(Serializer, testProcessInt64) {
  int64_t val = 16163232646464LL;
  int64_t ret = 0;

  {
    IoRange ioRange;
    Serializer<int64_t>().append(ioRange, val);

    auto range = ioRange.getRange();

    ASSERT_TRUE(Deserializer<int64_t>().parse(range, ret));
    ASSERT_EQ(val, ret);
    ASSERT_EQ(range.size(), 0);
  }

  ASSERT_EQ(Serializer<int64_t>().sizeOf(val), 8);

  {
    IoRange ioRange;
    Serializer<int64_t>().append(ioRange, val);
    auto range = ioRange.getRange();

    ASSERT_TRUE(Deserializer<int64_t>().skip(range));
    ASSERT_EQ(range.size(), 0);
  }
}

TEST(Serializer, testProcessUint16) {
  uint16_t val = 1616;
  uint16_t ret = 0;

  {
    IoRange ioRange;
    Serializer<uint16_t>().append(ioRange, val);

    auto range = ioRange.getRange();

    ASSERT_TRUE(Deserializer<uint16_t>().parse(range, ret));
    ASSERT_EQ(val, ret);
    ASSERT_EQ(range.size(), 0);
  }

  ASSERT_EQ(Serializer<uint16_t>().sizeOf(val), 2);

  {
    IoRange ioRange;
    Serializer<uint16_t>().append(ioRange, val);
    auto range = ioRange.getRange();

    ASSERT_TRUE(Deserializer<uint16_t>().skip(range));
    ASSERT_EQ(range.size(), 0);
  }
}

TEST(Serializer, testProcessUint32) {
  uint32_t val = 16163232;
  uint32_t ret = 0;

  {
    IoRange ioRange;
    Serializer<uint32_t>().append(ioRange, val);

    auto range = ioRange.getRange();

    ASSERT_TRUE(Deserializer<uint32_t>().parse(range, ret));
    ASSERT_EQ(val, ret);
    ASSERT_EQ(range.size(), 0);
  }

  ASSERT_EQ(Serializer<uint32_t>().sizeOf(val), 4);

  {
    IoRange ioRange;
    Serializer<uint32_t>().append(ioRange, val);
    auto range = ioRange.getRange();

    ASSERT_TRUE(Deserializer<uint32_t>().skip(range));
    ASSERT_EQ(range.size(), 0);
  }
}

TEST(Serializer, testProcessUint64) {
  uint64_t val = 16163232646464LL;
  uint64_t ret = 0;

  {
    IoRange ioRange;
    Serializer<uint64_t>().append(ioRange, val);

    auto range = ioRange.getRange();

    ASSERT_TRUE(Deserializer<uint64_t>().parse(range, ret));
    ASSERT_EQ(val, ret);
    ASSERT_EQ(range.size(), 0);
  }

  ASSERT_EQ(Serializer<uint64_t>().sizeOf(val), 8);

  {
    IoRange ioRange;
    Serializer<uint64_t>().append(ioRange, val);
    auto range = ioRange.getRange();

    ASSERT_TRUE(Deserializer<uint64_t>().skip(range));
    ASSERT_EQ(range.size(), 0);
  }
}

TEST(Serializer, testProcessDouble) {
  double val = 0.16163232646464;
  double ret = 0;

  {
    IoRange ioRange;
    Serializer<double>().append(ioRange, val);

    auto range = ioRange.getRange();

    ASSERT_TRUE(Deserializer<double>().parse(range, ret));
    ASSERT_EQ(val, ret);
    ASSERT_EQ(range.size(), 0);
  }

  ASSERT_EQ(Serializer<double>().sizeOf(val), 8);

  {
    IoRange ioRange;
    Serializer<double>().append(ioRange, val);
    auto range = ioRange.getRange();

    ASSERT_TRUE(Deserializer<double>().skip(range));
    ASSERT_EQ(range.size(), 0);
  }
}

TEST(Serializer, testProcessFloat) {
  float val = 0.16163232;
  float ret = 0;

  {
    IoRange ioRange;
    Serializer<float>().append(ioRange, val);

    auto range = ioRange.getRange();

    ASSERT_TRUE(Deserializer<float>().parse(range, ret));
    ASSERT_EQ(val, ret);
    ASSERT_EQ(range.size(), 0);
  }

  ASSERT_EQ(Serializer<float>().sizeOf(val), 4);

  {
    IoRange ioRange;
    Serializer<float>().append(ioRange, val);
    auto range = ioRange.getRange();

    ASSERT_TRUE(Deserializer<float>().skip(range));
    ASSERT_EQ(range.size(), 0);
  }
}

TEST(Serializer, testProcessString) {
  string val("hello, world");
  string ret;

  {
    IoRange ioRange;
    Serializer<string>().append(ioRange, val);

    auto range = ioRange.getRange();

    ASSERT_TRUE(Deserializer<string>().parse(range, ret));
    ASSERT_EQ(val, ret);
    ASSERT_EQ(range.size(), 0);
  }

  ASSERT_EQ(Serializer<string>().sizeOf(val), val.size() + 4);

  {
    IoRange ioRange;
    Serializer<string>().append(ioRange, val);
    auto range = ioRange.getRange();

    ASSERT_TRUE(Deserializer<string>().skip(range));
    ASSERT_EQ(range.size(), 0);
  }
}

TEST(Serializer, testProcessRange) {
  string str("hello, world");

  {
    string empty;
    Range val(str);
    Range ret(empty);

    IoRange ioRange;
    Serializer<Range>().append(ioRange, val);

    auto range = ioRange.getRange();

    ASSERT_TRUE(Deserializer<Range>().parse(range, ret));
    ASSERT_EQ(str, ret.toString());
    ASSERT_EQ(range.size(), 0);
  }

  {
    Range val(str);
    ASSERT_EQ(Serializer<Range>().sizeOf(val), val.size() + 4);
  }

  {
    Range val(str);
    IoRange ioRange;

    Serializer<Range>().append(ioRange, val);
    auto range = ioRange.getRange();

    ASSERT_TRUE(Deserializer<Range>().skip(range));
    ASSERT_EQ(range.size(), 0);
  }
}

TEST(Serializer, testProcessVector) {
  vector<int> val = {1, 2, 3};
  vector<int> ret;

  {
    IoRange ioRange;
    Serializer<vector<int>>().append(ioRange, val);

    auto range = ioRange.getRange();

    ASSERT_TRUE(Deserializer<vector<int>>().parse(range, ret));
    ASSERT_TRUE(val == ret);
    ASSERT_EQ(range.size(), 0);
  }

  ASSERT_EQ(Serializer<vector<int>>().sizeOf(val), val.size() * 4 + 4);

  {
    IoRange ioRange;
    Serializer<vector<int>>().append(ioRange, val);
    auto range = ioRange.getRange();

    ASSERT_TRUE(Deserializer<vector<int>>().skip(range));
    ASSERT_EQ(range.size(), 0);
  }
}

TEST(Serializer, testProcessMap) {
  map<int, int> val = {{1, 11}, {2, 12}};
  map<int, int> ret;

  {
    IoRange ioRange;
    Serializer<map<int, int>>().append(ioRange, val);

    auto range = ioRange.getRange();

    auto success = Deserializer<map<int, int>>().parse(range, ret);
    ASSERT_TRUE(success);

    ASSERT_TRUE(val == ret);
    ASSERT_EQ(range.size(), 0);
  }

  auto rsize = Serializer<map<int, int>>().sizeOf(val);
  ASSERT_EQ(rsize, val.size() * 8 + 4);

  {
    IoRange ioRange;
    Serializer<map<int, int>>().append(ioRange, val);
    auto range = ioRange.getRange();
    auto success = Deserializer<map<int, int>>().skip(range);
   
    ASSERT_TRUE(success);
    ASSERT_EQ(range.size(), 0);
  }
}

TEST(Serializer, testProcessUnorderedMap) {
  unordered_map<int, int> val = {{1, 11}, {2, 12}};
  unordered_map<int, int> ret;

  {
    IoRange ioRange;
    Serializer<unordered_map<int, int>>().append(ioRange, val);

    auto range = ioRange.getRange();

    auto success = Deserializer<unordered_map<int, int>>().parse(range, ret);
    ASSERT_TRUE(success);

    ASSERT_TRUE(val == ret);
    ASSERT_EQ(range.size(), 0);
  }

  auto rsize = Serializer<unordered_map<int, int>>().sizeOf(val);
  ASSERT_EQ(rsize, val.size() * 8 + 4);

  {
    IoRange ioRange;
    Serializer<unordered_map<int, int>>().append(ioRange, val);
    auto range = ioRange.getRange();
    auto success = Deserializer<unordered_map<int, int>>().skip(range);
   
    ASSERT_TRUE(success);
    ASSERT_EQ(range.size(), 0);
  }
}
