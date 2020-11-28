#include "comparators.h"


bool dir_sort(MyStats& a, MyStats& b) {
    return (S_ISDIR(a.file_stats.st_mode) && !S_ISDIR(b.file_stats.st_mode));
}

bool special_files_sort(MyStats& a, MyStats& b) { // check if works
    return ((!S_ISREG(a.file_stats.st_mode) && !S_ISDIR(a.file_stats.st_mode))
            && (S_ISREG(b.file_stats.st_mode) || S_ISDIR(b.file_stats.st_mode)));
}

bool sort_by_name(MyStats& a, MyStats& b) { return a.name < b.name; }

bool sort_by_size(MyStats& a, MyStats& b) { return a.file_stats.st_size < b.file_stats.st_size; }

std::string get_file_extention(const std::string& FileName) {
    if(FileName.find_last_of(".") != std::string::npos)
        return FileName.substr(FileName.find_last_of(".")+1);
    return "";
}

bool sort_by_extention(MyStats& a, MyStats& b) {
    return get_file_extention(a.name) < get_file_extention(b.name);
}

bool sort_by_time(MyStats& a, MyStats& b) { return a.file_stats.st_mtime < b.file_stats.st_mtime; }

std::map<std::string, bool(*)(MyStats&, MyStats&)> _comparators = {{"S", sort_by_size},
                                                                   {"t", sort_by_time},
                                                                   {"X", sort_by_extention},
                                                                   {"N", sort_by_name}};
