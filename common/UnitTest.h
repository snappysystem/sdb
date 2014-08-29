#ifndef COMMON_UNITTEST_H
#define COMMON_UNITTEST_H

#include <vector>
#include <string>
#include <functional>

namespace sdb {

struct TestInfo {

  std::string name;
  std::function<void()> fn;

  static std::vector<TestInfo>& getList();

  static bool registerTest(
    const std::string& name, const std::function<void()>& cob);

};


class MakeForkedProcess {
 public:

  void operator=(std::function<void()> cb);

};

}

#define ASSERT_EQ(x, y) if ((x) != (y)) {\
  auto xx = x; \
  auto yy = y; \
  LOG(FATAL) << "Expect value " << xx << " but got " << yy; }

#define ASSERT_GE(x, y) if ((x) < (y)) {\
  auto xx = x; \
  auto yy = y; \
  LOG(FATAL) << "Expect value " << xx << " ge " << yy; }

#define ASSERT_GT(x, y) if ((x) <= (y)) {\
  auto xx = x; \
  auto yy = y; \
  LOG(FATAL) << "Expect value " << xx << " gt " << yy; }

#define ASSERT_LE(x, y) if ((x) > (y)) {\
  auto xx = x; \
  auto yy = y; \
  LOG(FATAL) << "Expect value " << xx << " le " << yy; }

#define ASSERT_LT(x, y) if ((x) >= (y)) {\
  auto xx = x; \
  auto yy = y; \
  LOG(FATAL) << "Expect value " << xx << " lt " << yy; }

#define ASSERT_TRUE(x) if (!(x)) {\
  LOG(FATAL) << "Expect value to be true!"; }

#define ASSERT_FALSE(x) if (x) {\
  LOG(FATAL) << "Expect value to be false!"; }


/// Declare a unit test
#define TEST(x, y) \
  void x ## y(); \
  struct x ## y ## Test { \
    static bool val; \
  }; \
  bool x ## y ## Test::val =\
    sdb::TestInfo::registerTest(#x "::" #y, []() -> void { x ## y(); }); \
  void x ## y()


/// run a block of code in a forked child process
#define FORK sdb::MakeForkedProcess() = [&]()

#endif // COMMON_UNITTEST_H
