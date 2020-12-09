#include "parse_options.h"


bool check_if_wildcard(const std::string& str) {
    bool star = false;
    bool bracket = false;
    for (auto& c : str) {
        if (c == '?') { return true; }
        else if (c == '*') { star = true; }
        else if (star && (c != '/')) { return true; }
        else if (c == '[') { bracket = true; }
        else if (c == ']' && bracket) { return true; }
    }
    return false;
}

void find_wildcards_matches(const std::string& file, std::vector<std::string>& args) {
    glob_t glob_result;
    memset(&glob_result, 0, sizeof(glob_result));

    int return_value = glob(file.c_str(), GLOB_TILDE, nullptr, &glob_result);
    if(return_value != 0) {
        std::cerr << "wildcards: no matches found for " << file << std::endl;
        globfree(&glob_result);
    }
    for(size_t j = 0; j < glob_result.gl_pathc; ++j) {
        args.emplace_back(glob_result.gl_pathv[j]);
    }
    globfree(&glob_result);
}


void parse_options (int argc, char* argv[], OPTIONS& ls_args) {
    namespace po = boost::program_options;
    po::options_description visible("Supported options");
    visible.add_options()
            ("help,h", "Print help message.")
            ("is_detailed_info,l", "List detailed info about files.")
            ("reversed,r", "Print files in reversed order.")
            ("recursive,R", "Print all subdirectories entries recursively.")
            ("classify,F", "Print if file is :\n"
                           "* -- executable,\n"
                           "@ -- symlink,\n"
                           "| -- named chanel,\n"
                           "= -- socket,\n"
                           "? -- other.")
            ("sort", boost::program_options::value<std::string >()->composing()->default_value(
                    std::string {"N"}, "N"), "Define sorting :\n"
                         "U – [Unsorted], \n"
                         "S – sort by [Size], from smaller to bigger,\n"
                         "t – sort by modification [time], newest down, reversed to ls --sort=t.\n"
                         "X – sort by extension, \n"
                         "N – sort by name (default) [Name]. \n"
                         "With anyone from them can be one or two of the two following modifiers :\n"
                         "D -- [Directories_first],\n"
                         "s -- special files separate [special_file], first. If D -- after dirs.");

    po::options_description invisible("invisible opptions.");
    invisible.add_options()
            ("files",
             po::value<std::vector<std::string>>()->default_value(std::vector<std::string>{"."}, "."),
             "files for myls.");

    po::options_description all("All options");
    all.add(visible).add(invisible);
    po::positional_options_description positional;
    positional.add("files", -1);
    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(all).positional(positional).run(), vm);

    if (vm.count("help")) {
        std::cout << "Usage example :\n" <<
                  "\t./myls [path|mask] [-l] [-h|--help] [--sort=U|S|t|X|D|s] [-r]" << std::endl << std::endl;
        std::cout << visible << std::endl;
        exit(EXIT_SUCCESS);
    }

    if (vm.count("reversed") != 0) { ls_args.reversed_order = true; };
    if (vm.count("classify")) { ls_args.file_classify = true; }
    if (vm.count("recursive")) { ls_args.is_recursion = true; }
    if (vm.count("is_detailed_info")) {ls_args.is_detailed_info = true; }

    for (auto& c : vm["sort"].as<std::string>()) {
        if (c == 'U' || c == 't' || c == 'X' || c == 'N' || c== 'S') { ls_args.sort_by = c; }
        if (c == 'D') { ls_args.dir_first = true; }
        if (c == 's') { ls_args.sp_files_first = true; }
    }

    for (auto& file : vm["files"].as<std::vector<std::string >>()){
        if (check_if_wildcard(file)) {
            find_wildcards_matches(file, ls_args.files);
        } else if (boost::starts_with(file, "\"") && boost::ends_with(file, "\"")) {
            ls_args.files.push_back(file.substr(1, file.size()-2));
        } else {
            ls_args.files.push_back(file);
        }
    }
}
