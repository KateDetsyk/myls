#include <iostream>
#include <vector>
#include <boost/filesystem.hpp>
#include <ftw.h>
#include <iomanip>
#include <pwd.h>

#include "parse_options.h"
#include "comparators.h"
#include "structs.h"

OPTIONS ls_args{}; // global because we need it in func get_info()

std::map<std::string, std::vector<MyStats>> ALL_FILES;


void print_file(MyStats& entry) {
    std::stringstream ss;

    if (ls_args.is_detailed_info) {
        //permissions
        std::vector<int> permission_bits = { S_IRUSR, S_IWUSR, S_IXUSR, S_IRGRP, S_IWGRP,
                                             S_IXGRP, S_IROTH, S_IWOTH, S_IXOTH };
        std::vector<std::string> permissions = {"r", "w", "x", "r", "w", "x", "r", "w", "x"};
        std::string pass = "-";
        for (int i = 0; i < permission_bits.size(); ++i) {
            (entry.file_stats.st_mode & permission_bits[i]) ? ss << permissions[i] : ss << pass;
        }
        //owner
        struct passwd *pw = getpwuid(entry.file_stats.st_uid);
        ss << " " << std::setw(11) << pw->pw_name;

        // size
        ss << " " << std::setw(8) << (intmax_t) entry.file_stats.st_size;

        // date
        char date[20];
        strftime(date, sizeof(date), "%Y-%m-%d %H:%M:%S", gmtime(&(entry.file_stats.st_ctime)));
        ss << " " << std::setw(20) << date;
        ss << "  ";
    }

    if(S_ISDIR(entry.file_stats.st_mode)) { ss << "/"; }

    if(ls_args.file_classify) {
        if (S_ISDIR(entry.file_stats.st_mode)){}
        else if ((entry.file_stats.st_mode & S_IEXEC) != 0){
            ss << "*";
        } else if(S_ISLNK(entry.file_stats.st_mode)){
            ss << "@";
        } else if(S_ISFIFO(entry.file_stats.st_mode)) {
            ss << "|";
        } else if(S_ISSOCK(entry.file_stats.st_mode)) {
            ss << "=";
        } else if (!S_ISREG(entry.file_stats.st_mode)){
            ss << "?";
        }
    }

    ss << entry.name << "  ";
    (ls_args.is_detailed_info) ? ss << std::endl : ss << "  ";
    std::cout << ss.str();
}

static int get_info(const char *fpath, const struct stat *st, int tflag, struct FTW *ftwbuf) {
    // The typeflag argument (3rd) passed to fn() is an integer that has one of
    // the following values: FTW_F  fpath is a regular file. FTW_D  fpath is a directory.....

    if (ftwbuf->level > 1 && !ls_args.is_recursion) { return FTW_SKIP_SUBTREE; } //skip subdirectories if no recursion

    if (tflag == FTW_DNR) { std::cerr << "myls: No permission to read " << fpath << std::endl; }
    std::string path {fpath};
    if (tflag == FTW_F || ftwbuf->level > 0) {
        std::string dirname = path.substr(0, path.find_last_of(basename(fpath)) - strlen(basename(fpath))); ////////
        std::string filename = basename(fpath);
        MyStats entry = MyStats(filename, *st);
        ALL_FILES[dirname].push_back(entry);
    }
    return 0;
}

//ToDo: mask
//ToDo: other sorting
//ToDo: info for file not folder +++

int main(int argc, char* argv[]) {
    //parse options && fill options structure
    parse_options(argc, argv, ls_args);

    // print all for the first target than for the second and so on.
    for (auto& file : ls_args.files) {
        // nopenfd = 1 (3rd argument) specifies the maximum number of directories that nftw() will hold open simultaneously.
        if (nftw(file, get_info, 1, FTW_MOUNT | FTW_PHYS | FTW_DEPTH | FTW_ACTIONRETVAL) == -1) {
            std::cerr << "myls: Incorrect argument. No such file or directory " << file << std::endl;
        } else {
            for (auto &dir : ALL_FILES) {
                // sort files
//                sort(dir.second.begin(), dir.second.end(), comparator);
                sort(dir.second.begin(), dir.second.end(), _comparators[ls_args.sort_by]);
                //ToDo: check if sort special files first * @ = ?
                if (ls_args.sp_files_first) { sort(dir.second.begin(), dir.second.end(), special_files_sort); }
                if (ls_args.dir_first) { sort(dir.second.begin(), dir.second.end(), dir_sort); }

                // we print "pasth/to/dir: " if we call ls for few files or if we have recursion
                if ((ls_args.files.size() > 1) || (ls_args.is_recursion)) {
                    std::string base_filename = dir.first.substr(dir.first.find_last_of("/\\") + 1);

                    // if equal then target is just file for what we need to show info
                    if (base_filename != dir.second[0].name) {
                        std::cout << dir.first << ":" << std::endl;
                    }
                }
                for (auto &entry : dir.second) {
                    print_file(entry);
                }
                std::cout << std::endl;
            }
        }
        ALL_FILES.clear(); //clean vector for the next target
    }
    return 0;
}