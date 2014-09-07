#include "server/Socket.h"
#include "common/ThreadPool.cpp"
#include "common/UnitTest.cpp"

using namespace std;
using namespace sdb;


TEST(Socket, testGetLocalAddresses) {
  auto ret = Socket::getLocalAddresses();
  ASSERT_GT(ret.size(), 0);

  for (auto& p : ret) {
    ASSERT_TRUE(p.second == Socket::sock_v4 || p.second == Socket::sock_v6);
    ASSERT_FALSE(p.first.empty());
  }
}

TEST(Socket, testV4Creation) {
  Socket s(-1, Socket::sock_v4);
  ASSERT_GE(s.getFD(), 0);
  s.close();
}

TEST(Socket, testV6Creation) {
  Socket s(-1, Socket::sock_v6);
  ASSERT_GE(s.getFD(), 0);
  s.close();
}

TEST(Socket, testBindFreePort) {
  Socket s;
  int port = s.bindToFreePort("127.0.0.1");
  ASSERT_GE(port, 0);
  s.close();
}

TEST(Socket, testConnection) {
  FORK {
    auto tp = new ThreadPool(1);
    auto fut = tp->async([]() {
      pair<int, int> sockAndPort;
      Socket s;

      sockAndPort.first = s.getFD();
      sockAndPort.second = s.bindToFreePort("127.0.0.1");

      s.listen(10);
      return sockAndPort;
    });

    auto sockAndPort = fut.get();
    int socket = sockAndPort.first;

    auto done = tp->async([socket]() {
      Socket s(socket);
      int fd = s.accept();
      Socket newSocket(fd);
      newSocket.close();
      return true;
    });

    Socket cli;
    cli.connect("127.0.0.1", sockAndPort.second);

    done.wait();

    cli.close();
    Socket serv(sockAndPort.first);
    serv.close();

    tp->drain();

    delete tp;
  };
}

TEST(Socket, testNonblocking) {
  FORK {
    auto tp = new ThreadPool(1);
    auto fut = tp->async([]() {
      pair<int, int> sockAndPort;
      Socket s;

      sockAndPort.first = s.getFD();
      sockAndPort.second = s.bindToFreePort("127.0.0.1");

      return sockAndPort;
    });

    auto sockAndPort = fut.get();

    Socket cli;

    cli.setNonblocking(true);
    ASSERT_FALSE(cli.connect("127.0.0.1", sockAndPort.second));

    cli.close();
    Socket serv(sockAndPort.first);
    serv.close();

    tp->drain();

    delete tp;
  };
}

