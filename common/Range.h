#ifndef COMMON_RANGE_H
#define COMMON_RANGE_H

#include <vector>
#include <string>
#include <atomic>
#include <cstdint>
#include <algorithm>

namespace sdb {

// A general range of pointers that can be used to denote a piece
// of memory. It is possible to use this class to represent an array
// of POD types. But usually, we use this to represent an array of
// bytes.
//
// The class does not manage the memory of underlying data. It only
// serves as a reference to the memory region.
template <class T>
class basic_range {
 public:

  basic_range(T* beg, T* end) : beg_(beg), end_(end) {}

  basic_range(const basic_range<T>& another)
    : beg_(another.beg_), end_(another.end_) {}

  explicit basic_range(std::vector<T>& vec) {
    if (!vec.empty()) {
      beg_ = &vec[0];
      end_ = &vec[vec.size()];
    } else {
      beg_ = end_ = nullptr;
    }
  }

  explicit basic_range(std::basic_string<T>& str) {
    if (!str.empty()) {
      beg_ = &str[0];
      end_ = &str[str.size()];
    } else {
      beg_ = end_ = nullptr;
    }
  }

  basic_range(std::vector<T>& vec, uint64_t startPos, uint64_t endPos)
    : beg_(&vec[startPos]), end_(&vec[endPos]) {}

  basic_range(std::basic_string<T>& str, uint64_t startPos, uint64_t endPos)
    : beg_(&str[startPos]), end_(&str[endPos]) {}

  // return total size of the range
  int64_t size() const { return (end_ - beg_); }

  T* begin() { return beg_; }

  const T* begin() const { return beg_; }

  T* end() { return end_; }

  const T* end() const { return end_; }

  T& operator[](int64_t idx) { return beg_[idx]; }

  const T& operator[](int64_t idx) const { return beg_[idx]; }

  // find another range (substring), return the start pointer
  T* find(const basic_range<T>& another) {
    return std::search(beg_, end_, another.beg_, another.end_);
  }

  // find another range (substring), return the start pointer
  const T* find(const basic_range<T>& another) const {
    return std::search(beg_, end_, another.beg_, another.end_);
  }

  bool operator==(const basic_range<T>& another) const {
    if (size() != another.size()) {
      return false;
    } else {
      auto it = std::mismatch(begin(), end(), another.begin());
      return (it.first == end());
    }
  }

  bool operator!=(const basic_range<T>& another) const {
    return !(*this == another);
  }

  bool operator<(const basic_range<T>& another) const {
    return std::lexicographical_compare(
      begin(), end(), another.begin(), another.end());
  }

  // remove @nBytes from the beginning of the range
  void pop_front(int nBytes) {
    beg_ += nBytes;
    if (beg_ > end_) {
      beg_ = end_;
    }
  }

  // remove @nBytes from the end of the range
  void pop_back(int nBytes) {
    end_ -= nBytes;
    if (beg_ > end_) {
      end_ = beg_;
    }
  }

  // return a std::string representation
  std::string toString() const {
    return std::string(beg_, end_);
  }

 private:

  T* beg_;
  T* end_;
};


// a type for IoRange to indicate write mode
struct io_write_t {};

// a type for IoRange to indicate read mode
struct io_read_t {};


// A class to manage an IO buffer.
// Multiple IoRanges can refer to the same underlying buffer.
// The buffer will be reclaimed only if all references to it are gone.
// A typical usage of this class include:
//  (a) create a buffer;
//  (b) write (append) something to the buffer
//  (c) read whatever has written back from the buffer
class IoRange {
 public:

  struct shared_buf {
    char* buf;
    int size;
    std::atomic<int> refCount;

    shared_buf();
    ~shared_buf();

    void resize(const basic_range<char>& range);
  };

 public:

  // create a new IoRange, initialize a new shared buffer for write
  IoRange();

  // create a copy of original io range
  explicit IoRange(IoRange& another);

  // construct a range on top of another for write
  IoRange(io_write_t, IoRange& another, int startPos = 0);

  // construct a range on top of another for read
  IoRange(io_read_t, IoRange& another, int startPos, int endPos);

  ~IoRange();

  // return current range in iobuf. If the buffer is still in write mode,
  // this returns the range that has been written so far. If the buffer
  // is in read mode, it return the range that has not been read yet
  basic_range<char> getRange() {
    return {b_->buf + startPos_, b_->buf + endPos_};
  }

  void append(const char* beg, const char* end);

  void append(const basic_range<char>& range) {
    append(range.begin(), range.end());
  }

  void append(const std::string& str) {
    append(str.c_str(), str.c_str() + str.size());
  }

  void append(const std::vector<char>& data) {
    append(&data[0], &data[data.size()]);
  }

  char* begin() { return &b_->buf[startPos_]; }

  const char* begin() const { return &b_->buf[startPos_]; }

  char* end() { return &b_->buf[endPos_]; }

  const char* end() const { return &b_->buf[endPos_]; }

  // skip @numBytes from read buffer, return the number of bytes
  // actually skipped
  int skip(int numBytes);

  // read @numBytes from input buffer
  basic_range<char> read(int numBytes);

  // move remaining part to the beginning of the buffer if certain
  // condition (such as the remaining part is already at the very
  // end of a buffer) meets
  void relocate();

  // move remaining part to the beginning of the buffer unconditionally
  void forceRelocate();

 private:

  shared_buf* b_;

  int32_t startPos_;

  int32_t endPos_;
};


typedef basic_range<char> Range;

} // sdb

#endif // COMMON_RANGE_H
