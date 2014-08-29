from defs import *

cpp_unittest(
  name = "dir_test",
  srcs = [
    "DirTest.cpp",
  ],
  deps = [
    "common:libbase.a",
  ],
  linkopt = [
    "-pthread",
  ],
)

