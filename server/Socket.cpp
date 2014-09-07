#include "server/Socket.h"
#include "common/Logging.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>


using namespace std;

namespace sdb {

// A class to release addrinfo after exiting the scope
class AddrInfoPtr {
 public:

  typedef struct addrinfo* pointer_type;
  AddrInfoPtr() : ptr_(nullptr) {}
  ~AddrInfoPtr() {
    if (ptr_) {
      freeaddrinfo(ptr_);
    }
  }

  pointer_type& get() { return ptr_; }
  pointer_type operator->() { return ptr_; }

 private:

  pointer_type ptr_;

};


// A placeholder to allocate enough space for a socket address
class Sockaddr {
 public:

  Sockaddr() {}
  struct sockaddr* get() { return (struct sockaddr*)this; }
  struct sockaddr* operator->() { return get(); }
  int size() { return sizeof(*this); }

 private:

  char data_[sizeof(struct sockaddr_in6)];

};


Socket::Socket(int fd, int stype) : fd_(fd), sType_(stype) {
  if (fd < 0) {
    switch (stype) {
      case sock_v4:
        fd_ = socket(PF_INET, SOCK_STREAM, 0);
        break;

      case sock_v6:
        fd_ = socket(PF_INET6, SOCK_STREAM, 0);
        break;

      default:
        LOG(FATAL) << "Bad socket type " << stype;
        break;
    }
  }
}


string Socket::getHost() const {
  Sockaddr addr;
  socklen_t s = addr.size();

  if (0 != getsockname(fd_, addr.get(), &s)) {
    return string();
  }

  string ret(64, '\0');
  int rc = getnameinfo(
    addr.get(), addr.size(), &ret[0], ret.size(), nullptr, 0, NI_NUMERICHOST);

  if (rc != 0) {
    LOG(ERROR) << "getnameinfo() failed " << gai_strerror(rc);
    return string();
  }

  ret.resize(strlen(ret.c_str()));
  return ret;
}

int Socket::getPort() const {
  Sockaddr addr;
  socklen_t s = addr.size();

  if (0 != getsockname(fd_, addr.get(), &s)) {
    return -1;
  }

  string ret(64, '\0');
  int rc = getnameinfo(
    addr.get(), addr.size(), nullptr, 0, &ret[0], ret.size(), NI_NUMERICSERV);

  if (rc != 0) {
    LOG(ERROR) << "getnameinfo() failed " << gai_strerror(rc);
    return -1;
  }

  return stoi(ret);
}


bool Socket::setRecvTimeout(int64_t ms) {
  struct timeval s;
  socklen_t ol = sizeof(s);
  s.tv_sec = ms / 1000;
  s.tv_usec = (ms - s.tv_sec * 1000) * 1000;
  return (-1 != setsockopt(fd_, SOL_SOCKET, SO_RCVTIMEO, &s, ol));
}

int64_t Socket::getRecvTimeout(int64_t ms) const {
  struct timeval s;
  socklen_t ol = sizeof(s);
  socklen_t* optlen = &ol;
  s.tv_sec = s.tv_usec = 0;
  if (-1 == getsockopt(fd_, SOL_SOCKET, SO_RCVTIMEO, &s, optlen)) {
    return -1;
  }

  return (s.tv_sec * 1000 + s.tv_usec / 1000);
}


bool Socket::setSendTimeout(int64_t ms) {
  struct timeval s;
  socklen_t ol = sizeof(s);
  s.tv_sec = ms / 1000;
  s.tv_usec = (ms - s.tv_sec * 1000) * 1000;
  return (-1 != setsockopt(fd_, SOL_SOCKET, SO_SNDTIMEO, &s, ol));
}

int64_t Socket::getSendTimeout() const {
  struct timeval s;
  socklen_t ol = sizeof(s);
  socklen_t *optlen = &ol;
  s.tv_sec = s.tv_usec = 0;
  if (-1 == getsockopt(fd_, SOL_SOCKET, SO_SNDTIMEO, &s, optlen)) {
    return -1;
  }

  return (s.tv_sec * 1000 + s.tv_usec / 1000);
}

bool Socket::setSendBufSize(int64_t s) {
  return (-1 != setsockopt(fd_, SOL_SOCKET, SO_SNDBUF, &s, sizeof(s)));
}

int64_t Socket::getSendBufSize() const {
  size_t bufSize = 0;
  socklen_t ol;
  socklen_t *optlen = &ol;
  *optlen = sizeof(bufSize);
  if (-1 == getsockopt(fd_, SOL_SOCKET, SO_SNDBUF, &bufSize, optlen)) {
    return -1;
  }

  return bufSize;
}

bool Socket::setRecvBufSize(int64_t s) {
  return (-1 != setsockopt(fd_, SOL_SOCKET, SO_RCVBUF, &s, sizeof(s)));
}

int64_t Socket::getRecvBufSize() const {
  size_t bufSize = 0;
  socklen_t ol;
  socklen_t *optlen = &ol;
  *optlen = sizeof(bufSize);
  if (-1 == getsockopt(fd_, SOL_SOCKET, SO_RCVBUF, &bufSize, optlen)) {
    return -1;
  }

  return bufSize;
}


bool Socket::setReuseAddress() {
  int val = 1;
  return (-1 != setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)));
}

bool Socket::getReuseAddress() const {
  int v = 0;
  socklen_t ol;
  socklen_t *optlen = &ol;
  *optlen = sizeof(v);
  if (-1 == getsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &v, optlen)) {
    return false;
  }

  return (v > 0);
}

bool Socket::setLinger(int seconds) {
  struct linger l;
  if (seconds <= 0) {
    l.l_onoff = 0;
  } else {
    l.l_onoff = 1;
    l.l_linger = seconds;
  }

  return (-1 == setsockopt(fd_, SOL_SOCKET, SO_LINGER, &l, sizeof(l)));
}

int Socket::getLinger() const {
  struct linger l;
  socklen_t ol;
  socklen_t *optlen = &ol;
  *optlen = sizeof(l);

  if (-1 == getsockopt(fd_, SOL_SOCKET, SO_LINGER, &l, optlen)) {
    return -1;
  }

  return (l.l_onoff == 0) ? 0 : l.l_linger;
}

bool Socket::setNoDelay(bool noDelay) {
  int v = noDelay ? 1 : 0;
  return (-1 == setsockopt(fd_, IPPROTO_TCP, TCP_NODELAY, &v, sizeof(v)));
}

bool Socket::getNoDelay() const {
  int v = 0;
  socklen_t ol;
  socklen_t *optlen = &ol;
  *optlen = sizeof(v);

  if (-1 == getsockopt(fd_, IPPROTO_TCP, TCP_NODELAY, &v, optlen)) {
    return false;
  }

  return v;
}

bool Socket::setNonblocking(bool nonBlocking) {
  int flags = fcntl(fd_, F_GETFL, 0);
  if (-1 == flags) {
    return false;
  }

  if (nonBlocking) {
    if (flags & O_NONBLOCK) {
      return true;
    }

    if (-1 == fcntl(fd_, F_SETFL, flags | O_NONBLOCK)) {
      return false;
    }
  } else {
    if (!(flags & O_NONBLOCK)) {
      return true;
    }

    if (-1 == fcntl(fd_, F_SETFL, flags & ~O_NONBLOCK)) {
      return false;
    }
  }

  return true;
}

bool Socket::getNonblocking() const {
  int flags = fcntl(fd_, F_GETFL, 0);
  if (-1 == flags) {
    return false;
  }

  return (flags & O_NONBLOCK);
}

int Socket::getFD() const {
  return fd_;
}

bool Socket::bind(const std::string& ip, int port) {
  struct addrinfo hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  AddrInfoPtr pa;
  int error = getaddrinfo(
    ip.c_str(), to_string(port).c_str(), &hints, &pa.get());

  if (error != 0) {
    LOG(WARNING) << "Fails to resove address for: " << ip << " "
                 << gai_strerror(error);
    return false;
  }

  for (auto rp = pa.get(); rp != nullptr; rp = rp->ai_next) {
    if ((sType_ == sock_v4 && pa->ai_addr->sa_family == AF_INET) ||
        (sType_ == sock_v6 && pa->ai_addr->sa_family == AF_INET6)) {
      return (0 == ::bind(fd_, pa->ai_addr, pa->ai_addrlen));
    }
  }

  return false;
}

int Socket::bindToFreePort(const std::string& ip) {
  for (int port = 9000; port < 65535; ++port) {
    if (bind(ip, port)) {
      return port;
    }
  }
  return -1;
}

bool Socket::connect(const std::string& ip, int port) {
  struct addrinfo hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  AddrInfoPtr pa;
  int error = getaddrinfo(
    ip.c_str(), to_string(port).c_str(), &hints, &pa.get());

  if (error != 0) {
    LOG(WARNING) << "Fails to resove address for: " << ip << " "
                 << gai_strerror(error);
    return false;
  }

  for (auto rp = pa.get(); rp != nullptr; rp = rp->ai_next) {
    if ((sType_ == sock_v4 && pa->ai_addr->sa_family == AF_INET) ||
        (sType_ == sock_v6 && pa->ai_addr->sa_family == AF_INET6)) {
      return (0 == ::connect(fd_, pa->ai_addr, pa->ai_addrlen));
    }
  }

  return false;
}

bool Socket::listen(int backlog) {
  return (0 == ::listen(fd_, backlog));
}

int Socket::accept() {
  return ::accept(fd_, nullptr, nullptr);
}

void Socket::close() {
  ::close(fd_);
  fd_ = -1;
}

vector<pair<string, int>> Socket::getLocalAddresses() {
  vector<pair<string, int>> ret;
  struct addrinfo hints;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  AddrInfoPtr pa;
  int error = getaddrinfo(nullptr, "0", &hints, &pa.get());

  if (error != 0) {
    LOG(WARNING) << "Fails to resove address: " << gai_strerror(error);
    return ret;
  }

  for (auto rp = pa.get(); rp != nullptr; rp = rp->ai_next) {
    if (rp->ai_socktype == SOCK_STREAM) {
      string host(64, '\0');
      int rc = getnameinfo(
        rp->ai_addr,
        rp->ai_addrlen,
        &host[0],
        host.size(),
        nullptr,
        0,
        NI_NUMERICHOST);

      if (rc != 0) {
        LOG(ERROR) << "getnameinfo() failed " << gai_strerror(rc);
        continue;
      }

      host.resize(strlen(host.c_str()));

      switch (rp->ai_family) {
        case AF_INET:
          ret.emplace_back(host, sock_v4);
          break;

        case AF_INET6:
          ret.emplace_back(host, sock_v6);
          break;

        default:
          break;
      }
    }
  }

  return ret;
}

}
