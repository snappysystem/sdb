#ifndef SERVER_SOCKET_H
#define SERVER_SOCKET_H

#include <string>
#include <vector>
#include <cstdint>


namespace sdb {

class Socket {
 public:

  // Types of sockets
  enum {
    sock_v4 = 0,
    sock_v6,
  };

 public:

  // Attach to an existing socket or create a new socket
  explicit Socket(int fd = -1, int sockType = sock_v4);


  // Return the numeric ip string
  std::string getHost() const;

  int getPort() const;


  bool setRecvTimeout(int64_t ms);

  int64_t getRecvTimeout(int64_t ms) const;


  bool setSendTimeout(int64_t ms);

  int64_t getSendTimeout() const;


  bool setSendBufSize(int64_t s);

  int64_t getSendBufSize() const;


  bool setRecvBufSize(int64_t s);

  int64_t getRecvBufSize() const;


  bool setReuseAddress();

  bool getReuseAddress() const;


  // A value 0 means no linger (close immediately)
  bool setLinger(int seconds);

  int getLinger() const;


  bool setNoDelay(bool noDelay);

  bool getNoDelay() const;


  bool setNonblocking(bool nonblocking);

  bool getNonblocking() const;


  int getFD() const;


  bool bind(const std::string& ip, int port);

  // scan to find a free port on the host and bind to it.
  // Return the port number or -1 if no free port is found
  int bindToFreePort(const std::string& ip);

  bool connect(const std::string& ip, int port);

  // listen on a server socket
  bool listen(int backlog);

  // accept a new connection, returns the new fd or -1 if error occurs
  int accept();

  // close the socket. After this call the underlying fd_ becomes invalid
  void close();

  // Return all local addresses and their corresponding socket type
  static std::vector<std::pair<std::string, int>> getLocalAddresses();

 private:

  int fd_;

  int sType_;
};

}

#endif // SERVER_SOCKET_H
