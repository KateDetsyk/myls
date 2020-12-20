#ifndef MYLS_PARSE_OPTIONS_H
#define MYLS_PARSE_OPTIONS_H


#include <string>
#include <vector>
#include <iostream>
#include <glob.h>
#include <boost/program_options.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem.hpp>
#include <regex>

struct OPTIONS{
    bool is_recursion = false;
    bool reversed_order = false;
    bool is_detailed_info = false;
    bool file_classify = false;
    bool dir_first = false;
    bool sp_files_first = false;
    std::string sort_by = "N";
    std::vector<std::string> files{};
};

int parse_options (int argc, char* argv[], OPTIONS& ls_args);

#endif //MYLS_PARSE_OPTIONS_H
