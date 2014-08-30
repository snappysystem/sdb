from defs import *

cpp_library(
  name = "libbase.a",
  srcs = [
    "Dir.cpp",
    "ThreadPool.cpp",
    "Event.cpp",
    "UnitTest.cpp",
  ],
)
