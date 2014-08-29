#ifndef COMMON_DIR_H
#define COMMON_DIR_H

#include <string>

namespace sdb {

class Dir {
 public:

  static bool removeDirectories(const std::string& name);

  static bool createDirectories(const std::string& name);

};

}

#endif // COMMON_DIR_H
