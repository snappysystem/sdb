#include "common/Range.h"

#include <stdlib.h>
#include <string.h>

using namespace std;


namespace sdb {

const int kDefBufferSize = 32768;

IoRange::shared_buf::shared_buf()
  : buf((char*)malloc(kDefBufferSize)), size(kDefBufferSize), refCount(0) {
}

IoRange::shared_buf::~shared_buf() {
  free(buf);
}

void IoRange::shared_buf::resize(const Range& range) {
  size = size * 2;
  auto ptr = (char*)malloc(size);
  memcpy(ptr, range.begin(), range.size());
  swap(ptr, buf);
  free(ptr);
}


IoRange::IoRange()
  : b_(new shared_buf),
    startPos_(0),
    endPos_(0) {
}

IoRange::IoRange(IoRange& another)
  : b_(another.b_),
    startPos_(another.startPos_),
    endPos_(another.endPos_) {
  b_->refCount.fetch_add(1);
}

IoRange::IoRange(io_write_t, IoRange& another, int startPos)
  : b_(another.b_),
    startPos_(startPos),
    endPos_(startPos) {
  b_->refCount.fetch_add(1);
}

IoRange::IoRange(io_read_t, IoRange& another, int startPos, int endPos)
  : b_(another.b_),
    startPos_(startPos),
    endPos_(endPos) {
  b_->refCount.fetch_add(1);
}

IoRange::~IoRange() {
  auto ret = b_->refCount.fetch_sub(1);
  if (ret == 1) {
    delete b_;
  }
}

void IoRange::append(const char* beg, const char* end) {
  auto inc = end - beg;
  if (inc + endPos_ <= b_->size) {
    memcpy(&b_->buf[endPos_], beg, inc);
    endPos_ += inc;
  } else {
    b_->resize(getRange());
    append(beg, end);
  }
}

int IoRange::skip(int numBytes) {
  auto total = endPos_ - startPos_;
  if (total >= numBytes) {
    startPos_ += numBytes;
    return numBytes;
  } else {
    startPos_ = endPos_;
    return total;
  }
}

Range IoRange::read(int numBytes) {
  auto total = endPos_ - startPos_;
  if (total >= numBytes) {
    auto beg = &b_->buf[startPos_];
    startPos_ += numBytes;
    return {beg, beg + numBytes};
  } else {
    auto range = getRange();
    startPos_ = endPos_;
    return range;
  }
}

void IoRange::relocate() {
  if (startPos_ > b_->size / 2) {
    forceRelocate();
  }
}

void IoRange::forceRelocate() {
  auto range = getRange();
  bcopy(range.begin(), b_->buf, range.size());
  startPos_ = 0;
  endPos_ = range.size();
}

} // sdb
