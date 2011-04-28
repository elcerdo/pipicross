#include "helper.h"

#include <regex.h>
#include <iostream>

Matches matchRegex(const std::string &regex_str,const std::string &line_str) {
  regex_t regex;

  { // build regex
    int ret = regcomp(&regex,regex_str.c_str(),REG_EXTENDED);
    if (ret) {
      size_t nerrmsg = regerror(ret,&regex,NULL,0);
      char errmsg[nerrmsg];
      regerror(ret,&regex,errmsg,nerrmsg);
      std::cerr << "error while building regex : " << errmsg << std::endl;
      regfree(&regex);
      return Matches();
    }
  }

  { // match regex
    size_t nmatches = regex.re_nsub+1;
    regmatch_t matches[nmatches];

    int ret = regexec(&regex,line_str.c_str(),nmatches,&matches[0],0);
    if (ret) {
      regfree(&regex);
      return Matches();
    }

    Matches matches_str;
    for (int k=0; k<nmatches; k++) {
      const regmatch_t *match = &matches[k];
      size_t length = match->rm_eo-match->rm_so;
      matches_str.push_back(line_str.substr(match->rm_so,length));
    }
    regfree(&regex);
    return matches_str;
  }
}

