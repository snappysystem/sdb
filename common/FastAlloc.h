#ifndef COMMON_FASTALLOC_H
#define COMMON_FASTALLOC_H

#include "common/Logging.h"

#include <cstdint>
#include <atomic>
#include <mutex>
#include <vector>

#include <stdlib.h>

namespace std {

template <class T> struct shared_alloc_impl;


// A fast allocator for STL containers
//
// This allocator pre-allocates a chunk of size B bytes. Subsequent
// allocation request will use the remainings of this chunk first.
// If there is no or too little bytes available in the chunk, standard
// allocation (by calling malloc) is used.
//
// The allocator will NOT deallocate any space that has been allocated,
// until that the entire allocator is out of scope. At that time, all
// allocated memory will be reclaimed.
template <class T, class I = uint64_t, int B = 4096>
class FastAlloc {
 public:

  // minimal requirement of STL allocator
  typedef T value_type;

  // specify the alignment of allocation. By default, allocation shall
  // align on 8-bytes boundary. That is the reason that the default
  // type is a uint64_t. In special case, one can make alignment type
  // to be char or uint8_t to save internal fragmentations.
  typedef I alignment_type;

  // minimal requirement of STL allocator
  template <class U> struct rebind { typedef FastAlloc<U, I, B> other; };

  explicit FastAlloc(size_t nbytes = B);

  // minimal requirement of STL allocator
  template <class U> FastAlloc(const FastAlloc<U, I, B>& other);

  // minimal requirement of STL allocator
  T* allocate(size_t n);

  // minimal requirement of STL allocator
  void deallocate(T* p, size_t n) {}

 public:

  // reference counted allocation implementation
  shared_alloc_impl<I>* impl_;
};


template <class T, class I, class U, int B>
bool operator==(const FastAlloc<T, I, B>& a, const FastAlloc<U, I, B>& b) {
  return (a.impl_ == b.impl_);
}

template <class T, class I, class U, int B>
bool operator!=(const FastAlloc<T, I, B>& a, const FastAlloc<U, I, B>& b) {
  return (a.impl_ != b.impl_);
}


template <class T> struct shared_alloc_impl {
  T* begin;
  T* end;
  std::mutex mt;
  std::atomic<int> rc;
  std::vector<char*> overflows;

  explicit shared_alloc_impl(size_t nbytes) {
    auto num = (nbytes + sizeof(T) - 1) / sizeof(T);
    begin = (T*)malloc(num * sizeof(T));
    end = begin + num;
    overflows.push_back((char*)begin);
  }

  ~shared_alloc_impl() {
    for (auto p : overflows) {
      free(p);
    }
  }

  char* allocate(size_t nbytes) {
    lock_guard<mutex> l(mt);
    auto num = (nbytes + sizeof(T) - 1) / sizeof(T);
    auto next = begin + num;
    if (next < end) {
      auto ret = (char*)begin;
      begin = next;
      return ret;
    } else {
      auto ret = (char*)malloc(nbytes);
      overflows.push_back(ret);
      return ret;
    }
  }

  // when an allocator refers to this implementaion, the allocator should
  // lock it
  void lock() { ++rc; }

  // when an allocator is destructed, it should unlock the implementation
  // that it refers to
  void unlock() {
    auto ret = rc.fetch_sub(1);
    if (ret > 1) {
      return;
    } else if (ret < 1) {
      cerr << "derefencing below the base!" << endl;
      abort();
    } else {
      delete this;
    }
  }
};

template <class T, class I, int B>
FastAlloc<T, I, B>::FastAlloc(size_t nbytes)
  : impl_(new shared_alloc_impl<I>(nbytes)) {
}

template <class T, class I, int B>
template <class U>
FastAlloc<T, I, B>::FastAlloc(const FastAlloc<U, I, B>& other)
  : impl_(other.impl_) {
  impl_->lock();
}

template <class T, class I, int B>
T* FastAlloc<T, I, B>::allocate(size_t n) {
  return (T*)impl_->allocate(n * sizeof(T));
}

} // std

#endif // COMMON_FASTALLOC_H
