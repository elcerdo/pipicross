#ifndef __HELPER_H__
#define __HELPER_H__

#include <string>
#include <vector>
#include <sstream>
#include <cassert>

typedef std::vector<std::string> Matches;
Matches matchRegex(const std::string &regex_str,const std::string &line_str);

template <typename T>
T convertFromString(const std::string &str) {
  std::stringstream stream;
  stream.exceptions(std::stringstream::failbit|std::stringstream::badbit);
  stream << str;
  T value;
  stream >> value;
  return value;
}

#endif
