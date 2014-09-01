from defs import *

cpp_library(
  name = "libbase.a",
  srcs = [
    "Dir.cpp",
    "ThreadPool.cpp",
    "Event.cpp",
    "Range.cpp",
    "UnitTest.cpp",
  ],
)
