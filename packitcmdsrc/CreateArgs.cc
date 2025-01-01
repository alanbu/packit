#include "CreateArgs.h"

#include <iostream>

/// @brief Parse the arguments from the command line
/// @return true if parse was successful
bool CreateArgs::parse()
{
    if (_argc < 5)
    {
        std::cerr << "Too few argument for command" << std::endl;
        return false;
    }

    std::string arg;
    if (!get_next(arg) || arg != "create")
    {
        std::cerr << "First word on command line should be 'create'" << std::endl;
        return false;
    }
    if (!get_next(_output_folder))
    {
        std::cerr << "Output folder missing" << std::endl;
        return false;
    }
    if (!get_next(_control_file))
    {
        std::cerr << "Control file missing" << std::endl;
        return false;
    }

    if (!get_next(_copyright_file))
    {
        std::cerr << "Copyright file missing" << std::endl;
        return false;
    }

    while (get_next(arg))
    {
        bool arg_ok =- false;
        if (arg == "pack") arg_ok = parse_pack();
        else if (arg[0] == '-') arg_ok = parse_option(arg);
        else
        {
            std::string::size_type equals_pos = arg.find('=');
            if (equals_pos != std::string::npos)
            {
                _field_overrides[arg.substr(0,equals_pos)] = arg.substr(equals_pos+1);
                arg_ok = true;
            } else
            {
                std::cerr << "Invalid command line argument '" << arg << '"' << std::endl;
            }
        }
        if (!arg_ok) return false;
    }

    return true;
}

/// @brief Parse pack command line argment
/// @return true if successful
bool CreateArgs::parse_pack()
{
    std::string filename;
    if (!get_next(filename))
    {
        std::cerr << "Filename missing from pack argument" << std::endl;
        return false;
    }
    _files_to_pack.emplace_back(FileToPack());
    FileToPack &ftp = _files_to_pack.back();
    ftp.filename = filename;
    std::string opt_arg;
    if (!get_next(opt_arg))
    {
        return true;
    }
    if (opt_arg != "to")
    {
        restore_next();
        return true;
    }
    if (!get_next(ftp.pack_to))
    {
        std::cerr << "Pack to location missing from command line" << std::endl;
        return false;
    }
    if (!get_next(opt_arg))
    {
        return true;
    }
    if (opt_arg != "flags")
    {
        restore_next();
        return true;
    }
    if (!get_next(ftp.flags))
    {
        std::cerr << "Pack flags value missing from command line" << std::endl;
        return false;
    }

    return true;
}

/// @brief Parse option flags
/// @param option flag to check
/// @return true if flag is valid
bool CreateArgs::parse_option(const std::string &option)
{
    if (option == "--force" || option == "-f")
    {
        _force = true;
    } else if (option == "--check" || option == "-c")
    {
        _check = true;
    } else
    {
        std::cerr << "Invalid option " << option << std::endl;
        return false;
    }

    return true;
}

/// @brief Get next argument from the command line
/// @param value fetched value
/// @return true of argument fetched, false if at end of command line
bool CreateArgs::get_next(std::string &value)
{
    if (_index >= _argc) return false;
    value = _argv[_index++];
    return true;
}
