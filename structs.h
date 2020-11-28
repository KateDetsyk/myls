#ifndef MYLS_STRUCTS_H
#define MYLS_STRUCTS_H

#include <string>
#include <sys/stat.h>

struct MyStats{
    std::string name;
    struct stat file_stats{};
    MyStats(std::string& n, struct stat fs)
    {
        name = n;
        file_stats = fs;
    }
};

#endif //MYLS_STRUCTS_H
