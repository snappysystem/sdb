from defs import *

cpp_library(
  name = "libbase.a",
  srcs = [
    "Dir.cpp",
    "UnitTest.cpp",
  ],
)
