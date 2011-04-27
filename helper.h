#ifndef __HELPER_H__
#define __HELPER_H__

#include <string>
#include <vector>

typedef std::vector<std::string> Matches;
Matches match_regex(const std::string &regex_str,const std::string &line_str);

#endif
