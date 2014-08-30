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

cpp_unittest(
  name = "threadpool_test",
  srcs = [
    "ThreadPoolTest.cpp",
  ],
  deps = [
    "common:libbase.a",
  ],
  linkopt = [
    "-pthread",
  ],
)
