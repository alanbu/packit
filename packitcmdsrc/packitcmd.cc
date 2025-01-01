// packitcmd.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <string>
#include <cstring>
#include <algorithm>

#include "CreateArgs.h"
#include "Packager.h"

// Stop default unix lib file name processing
#include "unixlib/local.h"
int __riscosify_control = __RISCOSIFY_NO_PROCESS;

void usage();

namespace MainResult
{
    enum MainResultCode
    {
        PackageChecked = 1,
        PackageCreated = 0,
        InvalidArgument = -1,
        UnableToLoadControlFile = -2,
        UnableToLoadCopyrightFile = -3,
        NoItemsToPack = -4,
        InvalidControlField = -5,
        PackageDataError = -6,
        InvalidOutputFolder = -7,
        TargetDirectory = -8,
        TargetExists = -9,
        PackageCreateFailed = -10,
        SourceNotFound = -11
   };
}

/// @brief Main entry point for the program
/// @param argc count of number of arguments on the command line
/// @param argv pointers to arguments
/// @return 0 or error code.
int main(int argc, char *argv[])
{
    CreateArgs opts(argc, argv);
    if (!opts.parse())
    {
        usage();
        return MainResult::InvalidArgument;
    }

    Packager pkg;
    if (!pkg.load_control(opts.control_file()))
    {
        std::cerr << "Unable to load control file: " << opts.control_file() << std::endl;
        return MainResult::UnableToLoadControlFile;
    }

    if (!pkg.load_copyright(opts.copyright_file()))
    {
        std::cerr << "Unable to load copyright file: " << opts.copyright_file() << std::endl;
        return MainResult::UnableToLoadCopyrightFile;
    }

    const auto &files_to_pack = opts.files_to_pack();
    if (files_to_pack.empty())
    {
        std::cerr << "No items specified to add to the package" << std::endl;
        return MainResult::NoItemsToPack;
    }

    for (auto &pack_file : files_to_pack)
    {
        if (!tbx::Path(pack_file.filename).exists())
        {
            std::cerr << "File/folder to pack does not exists " << pack_file.filename << std::endl;
            return MainResult::SourceNotFound;
        }
        pkg.add_item_to_package(pack_file.filename, pack_file.pack_to, pack_file.flags);
    }

    const auto &field_overrides = opts.field_overrides();
    try
    {
        for (const auto &name_value : field_overrides)
        {
            pkg.set_control_field(name_value.first, name_value.second);
        }        
    }
    catch(const PackageFormatException& field_error)
    {
        std::cerr << "Command line field error " << field_error.message() << std::endl;
        return MainResult::InvalidControlField;
    }
    
    if (pkg.error_count())
    {
        std::cerr << "Unable to create package due to the following errors" << std::endl;
        auto error_list = pkg.errors();
        for(const std::string &error_text : error_list)
        {
            std::cerr << "  " << error_text << std::endl;
        }
        return MainResult::PackageDataError;
    }

    tbx::Path folder_path(opts.output_folder());
    if (!tbx::Path(folder_path).directory())
    {
        std::cerr << "Output folder '" << opts.output_folder() << "' does not exist or is not a directory" << std::endl;
        return MainResult::InvalidOutputFolder;
    }
    std::string leaf_name = pkg.package_name() + "_" + pkg.version() + "-" + pkg.package_version();
    // RISC OS can't handle "." in file names.
    std::replace(leaf_name.begin(), leaf_name.end(), '.', '/');
    tbx::Path full_path(folder_path, leaf_name);

    full_path.canonicalise();

    if (opts.check())
    {
        std::cout << "Checking control file only, '" << full_path << "' not created" << std::endl;
        return MainResult::PackageChecked;
    }

    if (full_path.exists())
    {
        if (full_path.directory())
        {
            std::cerr << "Output file '"<< full_path << "' is a directory and cannot be overwritten" << std::endl;
            return MainResult::TargetDirectory;
        }
        if (!opts.force())
        {
            std::cerr << "Output file '" << full_path << "' exists use the --force option to overwrite" << std::endl;
            return MainResult::TargetExists;
        }        
    }

    std::cout << "Creating package file '" << full_path << "'..." << std::endl;
    if (pkg.save(full_path))
    {
        std::cout << "package saved OK" << std::endl;
    } else
    {
        std::cout << "Package creation failed" << std::endl;
        return MainResult::PackageCreateFailed;
    }

    return MainResult::PackageCreated;
}


void usage()
{
    std::cout << "Usage: packit create <output_folder> <control_file> <copyright_file> {pack <file_or_dir> to <logical_dir> flags <flags>} [<fieldname>=<value>] [options]" << std::endl;    
    std::cout << "  Options are:" << std::endl;
    std::cout << "     --force -f    overwrite package file if it exists" << std::endl;
    std::cout << "     --check -c    do not write the package file, just check the control file created" << std::endl;
}
