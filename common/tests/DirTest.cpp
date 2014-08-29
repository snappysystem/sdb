#include "common/Logging.h"
#include "common/UnitTest.h"
#include "common/Dir.h"

#include <string>
#include <vector>

using namespace std;
using namespace sdb;


TEST(DirTest, testCreateAndRemove) {
  string s("/tmp//DirTest1/test/foo/bar");
  ASSERT_TRUE(Dir::createDirectories(s));
  ASSERT_TRUE(Dir::removeDirectories("/tmp/DirTest1"));
}

TEST(DirTest, testMultiCreateAndRemove) {
  vector<string> ss = {
    "/tmp/DirTest2/test/foo/bar",
    "/tmp/DirTest2/abc/xyz",
    "/tmp/DirTest2/pto/xyz",
    "/tmp/DirTest2/test/foo/final",
  };

  for (auto& s : ss) {
    ASSERT_TRUE(Dir::createDirectories(s));
  }

  ASSERT_TRUE(Dir::removeDirectories("tmp/DirTest2"));
}
