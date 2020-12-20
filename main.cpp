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
int STATUS = 0;
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

    //if few targets and no reqursion
//    if (ls_args.files.size() > 1 && ftwbuf->level != 0 && !ls_args.is_recursion) { return FTW_SKIP_SUBTREE; }
    if (ftwbuf->level > 1 && !ls_args.is_recursion) { return FTW_SKIP_SUBTREE; } //skip subdirectories if no recursion

    if (tflag == FTW_DNR) {
        std::cerr << "myls: No permission to read " << fpath << std::endl;
        STATUS = -1;
    }

    std::string path {fpath};
//    if ((ftwbuf->level == 0 && tflag == FTW_F) ||
//       (ls_args.files.size() > 1 && ftwbuf->level == 0 && !ls_args.is_recursion)) {
//        std::string filename = basename(fpath);
//        MyStats entry = MyStats(filename, *st);
//        ALL_FILES[filename].push_back(entry);
//    } else if (tflag == FTW_F || ftwbuf->level > 0) {
//        std::string filename = basename(fpath);
//        std::string dir_path = path.substr(0, path.size() - filename.size() - 1);
//        MyStats entry = MyStats(filename, *st);
//        ALL_FILES[dir_path].push_back(entry);
//    }

//    if ( ftwbuf->level == 0 && ls_args.is_mask) {
//        std::string filename = basename(fpath);
//        MyStats entry = MyStats(filename, *st);
//        ALL_FILES[filename].push_back(entry);
//    } else

//    if (ftwbuf->level == 0 && ls_args.is_mask && tflag == FTW_F ) {
//        return 0;
//    }
    if (tflag == FTW_F || ftwbuf->level > 0) {
        std::string filename = basename(fpath);
        std::string dir_path = path.substr(0, path.size() - filename.size() - 1);
        MyStats entry = MyStats(filename, *st);
        ALL_FILES[dir_path].push_back(entry);
    }
    return 0;
}


int main(int argc, char* argv[]) {
    //parse options && fill options structure
    STATUS = parse_options(argc, argv, ls_args);

    int flags = FTW_MOUNT | FTW_PHYS | FTW_DEPTH | FTW_ACTIONRETVAL;
    // print all for the first target than for the second and so on.
    for (auto& file : ls_args.files) {
        // nopenfd = 1 (3rd argument) specifies the maximum number of directories that nftw() will hold open simultaneously.
        if (nftw(const_cast<char *>(file.c_str()), get_info, 1, flags) == -1) {
            std::cerr << "myls: Incorrect argument. No such file or directory " << file << std::endl;
            STATUS = -1;
        } else {
            for (auto &dir : ALL_FILES) {
                // sort files
                //std::sort(v.begin(),v.end(),std::bind(ltMod,std::placeholders::_1,std::placeholders::_2,iMod));
                sort(dir.second.begin(), dir.second.end(), std::bind(_comparators[ls_args.sort_by],
                     std::placeholders::_1, std::placeholders::_2, ls_args.reversed_order));
                if (ls_args.sp_files_first) {
                    sort(dir.second.begin(), dir.second.end(), std::bind(special_files_sort,
                         std::placeholders::_1, std::placeholders::_2, ls_args.reversed_order));
                }
                if (ls_args.dir_first) {
                    sort(dir.second.begin(), dir.second.end(), std::bind(dir_sort,
                         std::placeholders::_1, std::placeholders::_2, ls_args.reversed_order));
                }

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

                (ls_args.is_detailed_info) ? std::cout << std::endl : std::cout << std::endl << std::endl;
            }
        }
        ALL_FILES.clear(); //clean vector for the next target
    }
    return STATUS;
}