#include "common/Logging.h"
#include "common/ThreadPool.h"
#include "common/UnitTest.h"
#include <iostream>

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>


using namespace std;


namespace sdb {

vector<TestInfo>& TestInfo::getList() {
  static vector<TestInfo> ret;
  return ret;
}

bool TestInfo::registerTest(
  const std::string& n, const std::function<void()>& cob) {
  TestInfo ti{n, cob};
  getList().push_back(ti);
  return true;
}


void MakeForkedProcess::operator=(function<void()> cb) {
  int pid = fork();
  if (pid < 0) {
    LOG(FATAL) << "Fails to run in child process!";
  } else if (pid > 0) {
    int status = 0;
    waitpid(pid, &status, 0);
    if (!WIFEXITED(status)) {
      LOG(FATAL) << "Child process crashed!";
    }
  } else {
    cb();
    exit(0);
  }
}

}

main() {
  auto& tests = sdb::TestInfo::getList();
  auto tp = new sdb::ThreadPool(4);

  mutex mt;

  for (auto& t : tests) {
    tp->submit([&t, &mt]() {
      {
        lock_guard<mutex> l(mt);
        cerr << "Running " << t.name << endl;
      }

      t.fn();
    });
  }

  tp->drain();
  delete tp;

  cerr << "Finished " << tests.size() << " tests." << endl;
}
