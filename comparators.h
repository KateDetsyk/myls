#ifndef MYLS_COMPARATORS_H
#define MYLS_COMPARATORS_H


#include <map>
#include <vector>
#include "structs.h"

bool dir_sort(MyStats& a, MyStats& b, bool reversed);

bool special_files_sort(MyStats& a, MyStats& b, bool reversed);

bool sort_by_name(MyStats& a, MyStats& b, bool reversed);

bool sort_by_extention(MyStats& a, MyStats& b, bool reversed);

bool sort_by_time(MyStats& a, MyStats& b, bool reversed);

bool sort_by_size(MyStats& a, MyStats& b, bool reversed);

extern std::map<std::string, bool(*)(MyStats&, MyStats&, bool reversed)> _comparators;

#endif //MYLS_COMPARATORS_H
