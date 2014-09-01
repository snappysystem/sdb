#ifndef COMMON_SERIALIZER_H
#define COMMON_SERIALIZER_H

#include "common/Range.h"

#include <string>
#include <vector>
#include <set>
#include <map>
#include <unordered_map>
#include <cstdint>

namespace sdb {

// Template classes to deserialize a string
template <class T> struct Deserializer;

// Specialization of Deserializer classes
template <> struct Deserializer<char> {
  bool parse(Range& range, char& val) const {
    if (range.size() > 0) {
      val = *range.begin();
      range.pop_front(1);
      return true;
    } else {
      return false;
    }
  }

  bool skip(Range& range) const {
    if (range.size() > 0) {
      range.pop_front(1);
      return true;
    } else {
      return false;
    }
  }
};

template <> struct Deserializer<bool> {
  bool parse(Range& range, bool& val) const {
    if (range.size() > 0) {
      val = (bool)*range.begin();
      range.pop_front(1);
      return true;
    } else {
      return false;
    }
  }

  bool skip(Range& range) const {
    if (range.size() > 0) {
      range.pop_front(1);
      return true;
    } else {
      return false;
    }
  }
};

template <> struct Deserializer<int16_t> {
  bool parse(Range& range, int16_t& val) const {
    if (range.size() > 1) {
      val = be16toh(*(int16_t*)range.begin());
      range.pop_front(2);
      return true;
    } else {
      return false;
    }
  }

  bool skip(Range& range) const {
    if (range.size() > 1) {
      range.pop_front(2);
      return true;
    } else {
      return false;
    }
  }
};

template <> struct Deserializer<uint16_t> {
  bool parse(Range& range, uint16_t& val) const {
    if (range.size() > 1) {
      val = be16toh(*(uint16_t*)range.begin());
      range.pop_front(2);
      return true;
    } else {
      return false;
    }
  }

  bool skip(Range& range) const {
    if (range.size() > 1) {
      range.pop_front(2);
      return true;
    } else {
      return false;
    }
  }
};

template <> struct Deserializer<int32_t> {
  bool parse(Range& range, int32_t& val) const {
    if (range.size() > 3) {
      val = be32toh(*(int32_t*)range.begin());
      range.pop_front(4);
      return true;
    } else {
      return false;
    }
  }

  bool skip(Range& range) const {
    if (range.size() > 3) {
      range.pop_front(4);
      return true;
    } else {
      return false;
    }
  }
};

template <> struct Deserializer<uint32_t> {
  bool parse(Range& range, uint32_t& val) const {
    if (range.size() > 3) {
      val = be32toh(*(uint32_t*)range.begin());
      range.pop_front(4);
      return true;
    } else {
      return false;
    }
  }

  bool skip(Range& range) const {
    if (range.size() > 3) {
      range.pop_front(4);
      return true;
    } else {
      return false;
    }
  }
};

template <> struct Deserializer<int64_t> {
  bool parse(Range& range, int64_t& val) const {
    if (range.size() > 7) {
      val = be64toh(*(int64_t*)range.begin());
      range.pop_front(8);
      return true;
    } else {
      return false;
    }
  }

  bool skip(Range& range) const {
    if (range.size() > 7) {
      range.pop_front(8);
      return true;
    } else {
      return false;
    }
  }
};

template <> struct Deserializer<uint64_t> {
  bool parse(Range& range, uint64_t& val) const {
    if (range.size() > 7) {
      val = be64toh(*(uint64_t*)range.begin());
      range.pop_front(8);
      return true;
    } else {
      return false;
    }
  }

  bool skip(Range& range) const {
    if (range.size() > 7) {
      range.pop_front(8);
      return true;
    } else {
      return false;
    }
  }
};

template <class T, class F>  T& bitwise_cast(F& val) {
  return *(T*)(&val);
}

template <> struct Deserializer<double> {
  bool parse(Range& range, double& val) const {
    return Deserializer<int64_t>().parse(
      range, bitwise_cast<int64_t, double>(val));
  }

  bool skip(Range& range) const {
    if (range.size() > 7) {
      range.pop_front(8);
      return true;
    } else {
      return false;
    }
  }
};

template <> struct Deserializer<float> {
  bool parse(Range& range, float& val) const {
    return Deserializer<int32_t>().parse(
      range, bitwise_cast<int32_t, float>(val));
  }

  bool skip(Range& range) const {
    if (range.size() > 3) {
      range.pop_front(4);
      return true;
    } else {
      return false;
    }
  }
};

template <> struct Deserializer<std::string> {
  bool parse(Range& range, std::string& val) const {
    int32_t len = 0;
    if (!Deserializer<int32_t>().parse(range, len)) {
      return false;
    }
    if (range.size() >= len) {
      val = std::string(range.begin(), len);
      range.pop_front(len);
      return true;
    } else {
      return false;
    }
  }

  bool skip(Range& range) const {
    int32_t len = 0;
    if (!Deserializer<int32_t>().parse(range, len)) {
      return false;
    }
    if (range.size() >= len) {
      range.pop_front(len);
      return true;
    } else {
      return false;
    }
  }
};

template <> struct Deserializer<Range> {
  bool parse(Range& range, Range& val) const {
    int32_t len = 0;
    if (!Deserializer<int32_t>().parse(range, len)) {
      return false;
    }
    if (range.size() >= len) {
      val = Range(range.begin(), range.begin() + len);
      range.pop_front(len);
      return true;
    } else {
      return false;
    }
  }

  bool skip(Range& range) const {
    int32_t len = 0;
    if (!Deserializer<int32_t>().parse(range, len)) {
      return false;
    }
    if (range.size() >= len) {
      range.pop_front(len);
      return true;
    } else {
      return false;
    }
  }
};

template <class F, class T> struct Deserializer<std::map<F, T>> {
  bool parse(Range& range, std::map<F, T>& val) const {
    int32_t num = 0;
    if (!Deserializer<int32_t>().parse(range, num)) {
      return false;
    }
    if (num < 0) {
      return false;
    }

    for (int i = 0; i < num; ++i) {
      std::pair<F, T> single;
      if (!Deserializer<F>().parse(range, single.first)) {
        return false;
      }
      if (!Deserializer<T>().parse(range, single.second)) {
        return false;
      }
      val.insert(single);
    }

    return true;
  }

  bool skip(Range& range) const {
    int32_t num = 0;
    if (!Deserializer<int32_t>().parse(range, num)) {
      return false;
    }
    if (num < 0) {
      return false;
    }
    for (int i = 0; i < num; ++i) {
      if (!Deserializer<F>().skip(range)) {
        return false;
      }
      if (!Deserializer<T>().skip(range)) {
        return false;
      }
    }

    return true;
  }
};

template <class F, class T> struct Deserializer<std::unordered_map<F, T>> {
  bool parse(Range& range, std::unordered_map<F, T>& val) const {
    int32_t num = 0;
    if (!Deserializer<int32_t>().parse(range, num)) {
      return false;
    }
    if (num < 0) {
      return false;
    }

    for (int i = 0; i < num; ++i) {
      std::pair<F, T> single;
      if (!Deserializer<F>().parse(range, single.first)) {
        return false;
      }
      if (!Deserializer<T>().parse(range, single.second)) {
        return false;
      }
      val.insert(single);
    }

    return true;
  }

  bool skip(Range& range) const {
    int32_t num = 0;
    if (!Deserializer<int32_t>().parse(range, num)) {
      return false;
    }
    if (num < 0) {
      return false;
    }
    for (int i = 0; i < num; ++i) {
      if (!Deserializer<F>().skip(range)) {
        return false;
      }
      if (!Deserializer<T>().skip(range)) {
        return false;
      }
    }

    return true;
  }
};

template <class F, class T> struct Deserializer<std::pair<F, T>> {
  bool parse(Range& range, std::pair<F, T>& val) const {
    if (!Deserializer<F>().parse(range, val.first)) {
      return false;
    }
    if (!Deserializer<T>().parse(range, val.second)) {
      return false;
    }
    return true;
  }

  bool skip(Range& range) const {
    if (!Deserializer<F>().skip(range)) {
      return false;
    }
    if (!Deserializer<T>().skip(range)) {
      return false;
    }
    return true;
  }
};

template <class T> struct Deserializer<std::vector<T>> {
  bool parse(Range& range, std::vector<T>& val) const {
    int32_t num = 0;
    if (!Deserializer<int32_t>().parse(range, num)) {
      return false;
    }
    if (num < 0) {
      return false;
    }

    val.resize(num);
    for (auto& item : val) {
      if (!Deserializer<T>().parse(range, item)) {
        return false;
      }
    }

    return true;
  }

  bool skip(Range& range) const {
    int32_t num = 0;
    if (!Deserializer<int32_t>().parse(range, num)) {
      return false;
    }
    if (num < 0) {
      return false;
    }

    for (int i = 0; i < num; ++i) {
      if (!Deserializer<T>().skip(range)) {
        return false;
      }
    }

    return true;
  }
};

template <class T> struct Deserializer<std::set<T>> {
  bool parse(Range& range, std::set<T>& val) const {
    int32_t num = 0;
    if (!Deserializer<int32_t>().parse(range, num)) {
      return false;
    }
    if (num < 0) {
      return false;
    }

    for (int i = 0; i < num; ++i) {
      T item;
      if (!Deserializer<T>().parse(range, item)) {
        return false;
      }

      val.insert(item);
    }

    return true;
  }

  bool skip(Range& range) const {
    int32_t num = 0;
    if (!Deserializer<int32_t>().parse(range, num)) {
      return false;
    }
    if (num < 0) {
      return false;
    }

    for (int i = 0; i < num; ++i) {
      if (!Deserializer<T>().skip(range)) {
        return false;
      }
    }

    return true;
  }
};


// Template class Serializer serializes a data structure T
// into a string
template <class T> struct Serializer;


// Specializations of Serializer classes
template <> struct Serializer<char> {
  void append(IoRange& ioRange, const char& val) const {
    ioRange.append(&val, &val + 1);
  }

  int sizeOf(const char& val) const {
    return 1;
  }
};

template <> struct Serializer<bool> {
  void append(IoRange& ioRange, const bool& val) const {
    Serializer<char>().append(ioRange, (const char&)val);
  }

  int sizeOf(const bool& val) const {
    return 1;
  }
};

template <> struct Serializer<int16_t> {
  void append(IoRange& ioRange, const int16_t& val) const {
    int16_t t = htobe16(val);
    ioRange.append((char*)&t, (char*)&t + 2);
  }

  int sizeOf(const int16_t& val) const {
    return 2;
  }
};

template <> struct Serializer<uint16_t> {
  void append(IoRange& ioRange, const uint16_t& val) const {
    uint16_t t = htobe16(val);
    ioRange.append((char*)&t, (char*)&t + 2);
  }

  int sizeOf(const uint16_t& val) const {
    return 2;
  }
};

template <> struct Serializer<int32_t> {
  void append(IoRange& ioRange, const int32_t& val) const {
    int32_t t = htobe32(val);
    ioRange.append((char*)&t, (char*)&t + 4);
  }

  int sizeOf(const int32_t& val) const {
    return 4;
  }
};

template <> struct Serializer<uint32_t> {
  void append(IoRange& ioRange, const uint32_t& val) const {
    uint32_t t = htobe32(val);
    ioRange.append((char*)&t, (char*)&t + 4);
  }

  int sizeOf(const uint32_t& val) const {
    return 4;
  }
};

template <> struct Serializer<int64_t> {
  void append(IoRange& ioRange, const int64_t& val) const {
    int64_t t = htobe64(val);
    ioRange.append((char*)&t, (char*)&t + 8);
  }

  int sizeOf(const int64_t& val) const {
    return 8;
  }
};

template <> struct Serializer<uint64_t> {
  void append(IoRange& ioRange, const uint64_t& val) const {
    uint64_t t = htobe64(val);
    ioRange.append((char*)&t, (char*)&t + 8);
  }

  int sizeOf(const uint64_t& val) const {
    return 8;
  }
};

template <class T, class F> const T& bitwise_const_cast(const F& val) {
  return *(const T*)(&val);
}

template <> struct Serializer<double> {
  void append(IoRange& ioRange, const double& val) const {
    Serializer<int64_t>().append(
      ioRange, bitwise_const_cast<int64_t, double>(val));
  }

  int sizeOf(const double& val) const {
    return 8;
  }
};

template <> struct Serializer<float> {
  void append(IoRange& ioRange, const float& val) const {
    Serializer<int32_t>().append(
      ioRange, bitwise_const_cast<int32_t, float>(val));
  }

  int sizeOf(const float& val) const {
    return 4;
  }
};

template <> struct Serializer<std::string> {
  void append(IoRange& ioRange, const std::string& val) const {
    int32_t len = val.size();
    Serializer<int32_t>().append(ioRange, len);
    ioRange.append(val);
  }

  int sizeOf(const std::string& val) const {
    int32_t len = val.size();
    return Serializer<int32_t>().sizeOf(len) + len;
  }
};

template <> struct Serializer<Range> {
  void append(IoRange& ioRange, const Range& val) const {
    int32_t len = val.size();
    Serializer<int32_t>().append(ioRange, len);
    ioRange.append(val);
  }

  int sizeOf(const Range& val) const {
    int32_t len = val.size();
    return Serializer<int32_t>().sizeOf(len) + len;
  }
};

template <class F, class T> struct Serializer<std::map<F, T>> {
  void append(IoRange& ioRange, const std::map<F, T>& val) const {
    Serializer<int32_t>().append(ioRange, val.size());

    for (auto& p : val) {
      Serializer<F>().append(ioRange, p.first);
      Serializer<T>().append(ioRange, p.second);
    }
  }

  int sizeOf(const std::map<F, T>& val) {
    int total = Serializer<int32_t>().sizeOf(val.size());
    for (auto& p : val) {
      total += Serializer<F>().sizeOf(p.first);
      total += Serializer<T>().sizeOf(p.second);
    }
    return total;
  }
};

template <class F, class T> struct Serializer<std::unordered_map<F, T>> {
  void append(IoRange& ioRange, const std::unordered_map<F, T>& val) const {
    Serializer<int32_t>().append(ioRange, val.size());

    for (auto& p : val) {
      Serializer<F>().append(ioRange, p.first);
      Serializer<T>().append(ioRange, p.second);
    }
  }

  int sizeOf(const std::unordered_map<F, T>& val) {
    int total = Serializer<int32_t>().sizeOf(val.size());
    for (auto& p : val) {
      total += Serializer<F>().sizeOf(p.first);
      total += Serializer<T>().sizeOf(p.second);
    }
    return total;
  }
};

template <class F, class T> struct Serializer<std::pair<F, T>> {
  void append(IoRange& ioRange, const std::pair<F, T>& val) const {
    Serializer<F>().append(ioRange, val.first);
    Serializer<T>().append(ioRange, val.second);
  }

  int sizeOf(const std::pair<F, T>& val) {
    int total = 0;
    total += Serializer<F>().sizeOf(val.first);
    total += Serializer<T>().sizeOf(val.second);
    return total;
  }
};

template <class T> struct Serializer<std::set<T>> {
  void append(IoRange& ioRange, const std::set<T>& val) const {
    Serializer<int32_t>().append(ioRange, val.size());

    for (auto& i : val) {
      Serializer<T>().append(ioRange, i);
    }
  }

  int sizeOf(const std::set<T>& val) const {
    int total = 0;

    total += Serializer<int32_t>().sizeOf(val.size());
    for (auto& i : val) {
      total += Serializer<T>().sizeOf(i);
    }

    return total;
  }
};

template <class T> struct Serializer<std::vector<T>> {
  void append(IoRange& ioRange, const std::vector<T>& val) const {
    Serializer<int32_t>().append(ioRange, val.size());

    for (auto& i : val) {
      Serializer<T>().append(ioRange, i);
    }
  }

  int sizeOf(const std::vector<T>& val) {
    int total = 0;
    total += Serializer<int32_t>().sizeOf(val.size());
    for (auto& i : val) {
      total += Serializer<T>().sizeOf(i);
    }

    return total;
  }
};

}  // sdb

#endif // COMMON_SERIALIZER_H
