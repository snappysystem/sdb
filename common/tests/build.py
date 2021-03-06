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

cpp_unittest(
  name = "event_test",
  srcs = [
    "EventTest.cpp",
  ],
  deps = [
    "common:libbase.a",
  ],
  linkopt = [
    "-pthread",
  ],
)

cpp_unittest(
  name = "range_test",
  srcs = [
    "RangeTest.cpp",
  ],
  deps = [
    "common:libbase.a",
  ],
)

cpp_unittest(
  name = "serializer_test",
  srcs = [
    "SerializerTest.cpp",
  ],
  deps = [
    "common:libbase.a",
  ],
)

cpp_unittest(
  name = "sharedmutex_test",
  srcs = [
    "SharedMutexTest.cpp",
  ],
  deps = [
    "common:libbase.a",
  ],
  linkopt = [
    "-pthread",
  ],
)

cpp_unittest(
  name = "fastalloc_test",
  srcs = [
    "FastAllocTest.cpp",
  ],
  deps = [
    "common:libbase.a",
  ],
)
