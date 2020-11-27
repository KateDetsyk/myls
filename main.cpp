#include <iostream>
#include <vector>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <ftw.h>

#include "parse_options.h"


struct MyStats{
    std::string name;
    struct stat file_stats{};
    MyStats(std::string& n, struct stat fs)
    {
        name = n;
        file_stats = fs;
    }
};

OPTIONS ls_args; // global because we need it in func get_info()

std::map<std::string, std::vector<MyStats>> DIRS;


static int get_info(const char *fpath, const struct stat *st, int tflag, struct FTW *ftwbuf) {
    // The typeflag argument (3rd) passed to fn() is an integer that has one of
    // the following values: FTW_F  fpath is a regular file. FTW_D  fpath is a directory.....

    if (ftwbuf->level > 1 && !ls_args.is_recursion) { return FTW_SKIP_SUBTREE; } //skip subdirectories if no recursion

    if (tflag == FTW_DNR) { std::cerr << "myls: No permission to read " << fpath << std::endl; }
//    std::cout << "PATH == " << fpath << std::endl;
    std::string path {fpath};
    if (tflag == FTW_D && ls_args.is_recursion) { //?????????????????????
        std::cout << "PATH == " << fpath << std::endl;
        MyStats entry = MyStats(path, *st);
        DIRS[fpath].push_back(entry);
    }
    if (ftwbuf->level > 0 || tflag == FTW_F) {
        std::string dirname = path.substr(0, path.find_last_of(basename(fpath)) - strlen(basename(fpath)));
        MyStats entry = MyStats(path, *st);
        DIRS[dirname].push_back(entry);
    }
    return 0;
}


void print_file(std::vector<MyStats>& entry) {

}

int ls() {
    // print all for the first target than for the second and so on.
    for (auto& file : ls_args.files) {
        // nopenfd = 1 (3rd argument) specifies the maximum number of directories that nftw() will hold open simultaneously.
        if (nftw(file, get_info, 1, FTW_MOUNT | FTW_PHYS | FTW_DEPTH | FTW_ACTIONRETVAL) == -1) {
            perror("nftw");
            exit(EXIT_FAILURE);
        } else {
            for (auto &dir : DIRS) {
                // we print "pasth/to/dir : " if we call ls for few files or if we have recursion
                if ((ls_args.files.size() > 1) || (ls_args.is_recursion && DIRS.size() > 1)) {
                    std::cout << dir.first << " :" << std::endl;
                }
//                std::cout << dir.first << " :" << std::endl;
                for (auto &entry : dir.second) {
                    std::string base_filename = entry.name.substr(entry.name.find_last_of("/\\") + 1);
                    std::cout << base_filename << "  ";
                }
                std::cout << std::endl << std::endl;
            }
        }
        std::cout << "---------------------------------------------------------------------" << std::endl;
        DIRS.clear();
    }
    return 0;
}


int main(int argc, char* argv[]) {
    //parse options && fill options structure
    parse_options(argc, argv, ls_args);

    //run ls
    ls();

    for (auto &dir : DIRS) {
        std::cout << dir.first << " :" << std::endl;
    }

    return 0;
}