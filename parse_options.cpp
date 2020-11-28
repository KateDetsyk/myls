#include "parse_options.h"


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
            ("sort", boost::program_options::value<std::vector<std::string >>()->composing()->default_value(
                    std::vector<std::string>{"N"}, "N"), "Define sorting :\n"
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
    auto parsed = po::command_line_parser(argc, argv).options(all).positional(positional).run();
    po::store(parsed, vm);

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

    for (auto& elem : vm["sort"].as<std::vector<std::string >>()) {
        for (auto& c : elem) {
            if (c == 'U' || c == 't' || c == 'X' || c == 'N' || c== 'S') { ls_args.sort_by = c; }
            if (c == 'D') { ls_args.dir_first = true; }
            if (c == 's') { ls_args.sp_files_first = true; }
        }
    }
    for (auto& file : vm["files"].as<std::vector<std::string >>()){
        ls_args.files.push_back(const_cast<char *>(file.c_str()));
    }
}
