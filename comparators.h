#ifndef MYLS_COMPARATORS_H
#define MYLS_COMPARATORS_H


#include <map>
#include <vector>
#include "structs.h"

bool dir_sort(MyStats& a, MyStats& b);

bool special_files_sort(MyStats& a, MyStats& b);

bool sort_by_name(MyStats& a, MyStats& b);

bool sort_by_extention(MyStats& a, MyStats& b);

bool sort_by_time(MyStats& a, MyStats& b);

bool sort_by_size(MyStats& a, MyStats& b);

extern std::map<std::string, bool(*)(MyStats&, MyStats&)> _comparators;

#endif //MYLS_COMPARATORS_H
