#ifndef INCLUDE_IO_WRAP_HPP
#define INCLUDE_IO_WRAP_HPP

#include <fstream>

namespace IOWrap {

template <typename T> void GetFromInput(T *arg, std::istream &input) {
  input.exceptions(std::istream::failbit | std::istream::badbit); // enable cin throw exceptions.
  input >> *arg;
}

template <typename T> void TryOpenFile(std::ifstream &input, const T &arg) {
  input.exceptions(std::ifstream::failbit | std::ifstream::badbit); // enable cin throw exceptions.
  input.open(arg);
}

} // namespace IOWrap

#endif