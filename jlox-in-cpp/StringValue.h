#pragma once

#include <cstring>
#include <limits>
#include <string_view>
#include <utility>

// This is a wrapper around a string view which is either unowned (someone else
// manages its lifetime) or reference counted. It also implements string
// concatenation, since we need that operation for our interpreter. The
// motivation is to avoid holding on to strings that didn't directly come from
// the parsed program (e.g. a new string we created via concatenation) when
// they're unneeded, to reduce memory usage. We aren't using shared_ptr because
// handling both the unowned and the ref-counted case would have been more work
// with that IMO, since I don't need atomicity or weak pointers.
class StringValue {
public:
  StringValue(std::string_view s) : data(s.data()), ownsData(false) {
    assert(s.length() <= std::numeric_limits<unsigned>::max());
    length = static_cast<unsigned>(s.length());
  }

  StringValue(const StringValue &s1, const StringValue &s2) {
    size_t combinedLength = s1.length + s2.length;
    assert(combinedLength >= s1.length); // check for overflow

    char *data = new char[sizeof(unsigned) + combinedLength];
    ownsData = true;
    std::memcpy(data + sizeof(unsigned), s1.data, s1.length);
    std::memcpy(data + sizeof(unsigned) + s1.length, s2.data, s2.length);
    length = combinedLength;

    // Point to string
    this->data = data + sizeof(unsigned);
    refCount() = 1;
  }

  StringValue(const StringValue &other)
      : data(other.data), length(other.length), ownsData(other.ownsData) {
    if (ownsData)
      ++refCount();
  }

  StringValue(StringValue &&other)
      : data(other.data), length(other.length), ownsData(other.ownsData) {
    other.ownsData = false;
  }

  StringValue &operator=(StringValue other) {
    // https://stackoverflow.com/a/3279550
    swap(*this, other);
    return *this;
  }

  ~StringValue() {
    if (ownsData && --refCount() == 0)
      delete[] (data - sizeof(unsigned));
  }

  bool operator==(const StringValue &other) const {
    return str() == other.str();
  }

  std::string_view str() const { return std::string_view(data, length); }

private:
  const char *data;
  unsigned length;
  bool ownsData;

  unsigned &refCount() {
    return *reinterpret_cast<unsigned *>(
        const_cast<char *>(data - sizeof(unsigned)));
  }

  friend void swap(StringValue &first, StringValue &second) {
    std::swap(first.data, second.data);
    std::swap(first.length, second.length);
    std::swap(first.ownsData, second.ownsData);
  }
};
