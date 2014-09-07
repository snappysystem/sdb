from defs import *

cpp_unittest(
  name = "socket_test",
  srcs = [
    "SocketTest.cpp",
  ],
  deps = [
    "common:libbase.a",
    "server:libserver.a",
  ],
  linkopt = [
    "-pthread",
  ],
)
