#pragma once

#include <string>
#include <vector>
#include <map>

class CreateArgs
{
    public:
       CreateArgs(int argc, char **argv) : _argc(argc), _argv(argv) {}

       bool parse();

       const std::string &output_folder() const {return _output_folder;}
       const std::string &control_file() const {return _control_file;}
       const std::string &copyright_file() const {return _copyright_file;}

       struct FileToPack
       {
        std::string filename;
        std::string pack_to;
        std::string flags;
       };
       const std::vector<FileToPack> &files_to_pack() const {return _files_to_pack;}
       const std::vector<std::string> &exclude() const {return _exclude;}
       const std::map<std::string,std::string> &field_overrides() const {return _field_overrides;}

       bool force() const {return _force;}
       bool check() const {return _check;}

    private:
       bool get_next(std::string &value);
       void restore_next() {--_index;}
       bool parse_pack();
       bool parse_option(const std::string &option);

    private:
       int _argc;
       char **_argv;
       int _index{1};
       std::string _output_folder;
       std::string _control_file;
       std::string _copyright_file;
       std::vector<FileToPack> _files_to_pack;
       std::map<std::string,std::string> _field_overrides;
       std::vector<std::string> _exclude;
       bool _force{false};
       bool _check{false};
};
