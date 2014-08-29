#ifndef COMMON_LOGGING_H
#define COMMON_LOGGING_H

#include <iostream>
#include <sstream>


namespace sdb {

class Logging {
 public:

  ~Logging() {
    auto s = ss_.str();
    std::cerr << s << std::endl;
    if (s[0] == 'F') {
      abort();
    }
  }

  template <class T>
  Logging& operator<<(const T& t) {
    ss_ << t;
    return *this;
  }

 private:

  std::stringstream ss_;
};

}

#define INFO "I"
#define WARNING "W"
#define ERROR "E"
#define FATAL "F"

#define LOG(x) Logging() << x << " " << __FILE__ << ":" << __LINE__ << " "

#endif // COMMON_LOGGING_H
