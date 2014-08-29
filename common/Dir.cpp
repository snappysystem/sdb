#include "common/Dir.h"
#include "common/Logging.h"

#include <string>
#include <sstream>

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>

using namespace std;

namespace sdb {

bool Dir::removeDirectories(const string& name) {
  struct dirent **namelist = nullptr;
  int n = scandir(name.c_str(), &namelist, nullptr, alphasort);

  if (n < 0) {
    if (namelist) {
      free(namelist);
    }
    if (errno == ENOENT) {
      return true;
    }
    LOG(ERROR) << "scandir " << name << " " << strerror(errno);
    return false;
  }

  while (n--) {
    string d_name = namelist[n]->d_name;
    auto d_type = namelist[n]->d_type;

    free(namelist[n]);

    if (d_name == string(".")) {
      continue;
    }
    if (d_name == string("..")) {
      continue;
    }
    string full = name + "/" + d_name;
    if (d_type == DT_DIR) {
      if (!removeDirectories(full)) {
        return false;
      }
    } else {
      if (0 > unlink(full.c_str())) {
        LOG(ERROR) << "unlink " << full << " " << strerror(errno);
        return false;
      }
    }
  }

  free(namelist);

  if (0 > rmdir(name.c_str())) {
    LOG(ERROR) << "rmdir " << name << " " << strerror(errno);
    return false;
  }

  return true;
}

bool Dir::createDirectories(const std::string& name) {
  string tmp;
  stringstream ss(name);

  while (!ss.eof()) {
    string s;
    getline(ss, s, '/');

    if (!s.empty()) {
      tmp += "/" + s;
      struct stat st;
      if (0 > stat(tmp.c_str(), &st)) {
        if (errno == ENOENT) {
          if (0 > mkdir(tmp.c_str(), 0777)) {
            LOG(ERROR) << "mkdir " << tmp << " " << strerror(errno);
            return false;
          }
        } else {
          LOG(ERROR) << "stat " << tmp << " " << strerror(errno);
          return false;
        }
      }
    }
  }

  return true;
}

} // sdb
