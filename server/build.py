from defs import *

cpp_library(
  name = "libserver.a",
  srcs = [
    "Socket.cpp",
  ],
  deps = [
    "common:libbase.a",
  ],
)
