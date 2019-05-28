/*********************************************************************
* Copyright 2009-2019 Alan Buckley
*
* This file is part of PackIt.
*
* PackIt is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* PackIt is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with PackIt. If not, see <http://www.gnu.org/licenses/>.
*
*****************************************************************************/



#include "Packager.h"
#include "Binding.h"
#include <fstream>
#include <iostream>
#include <algorithm>

#include "tbx/reporterror.h"
#include "tbx/path.h"
#include "tbx/stringutils.h"
#include "tbx/hourglass.h"

#define _ZIP_SYSTEM_LINUX
#include "ziparchive/ZipArchive.h"
#include "ziparchive/ZipException.h"
#include "RISCOSZipExtra.h"

Packager *Packager::_first_packager = 0;

/**
 * Name of package items, must be matched with PackageItem enum
 */
const char *Packager::_item_names[NUM_ITEMS] = {
  "Package name",
  "Version",
  "Package version",
  "Section",
  "Priority",
  "Maintainer",
  "Standards version",
  "Environment",
  "Install priority",
  "Summary",
  "Description",
  "Homepage",
  "Licence",
  "Copyright",
  "Item to install",
  "Install to",
  "Depends",
  "Recommends",
  "Suggests",
  "Conflicts",
  "OSDepends",
  "Components"
};

/**
 * Special directories found in the package
 */
const int NumSpecialDirs = 9;
enum SpecialDirId {SD_RISCPKG, SD_SYSVARS, SD_SPRITES,
      SD_APPS, SD_MANUALS, SD_RESOURCES, SD_SYSTEM,
      SD_TOBELOADED, SD_TOBETASKS,
      SD_NONE=99};

const char *SpecialDirs[NumSpecialDirs] =
{
	"RiscPkg",
	"SysVars",
	"Sprites",
	"Apps",
	"Manuals",
	"Resources",
	"System",
	"ToBeLoaded",
	"ToBeTasks"
};

/** Buffer for file copy **/
static int copy_buffer_size = 640 * 1024;
static char *copy_buffer = new char[copy_buffer_size];

/** Exception thrown if there is a problem creating a packages */
class PackageCreateException
{
	std::string message;
public:
	PackageCreateException(const std::string &msg) : message(msg) {};

	const std::string &what() const {return message;}
};


Packager::Packager() :
	_modified(false), _modified_listener(0),
	_error_count(0), _listener(0)
{
    package_name("");
    version("");
    package_version("1");
    section("");
    priority("Optional");
    maintainer("");
    environment("any");
    standards_version("0.4.0");
    install_priority("0");
    summary("");
    licence("");
    copyright("");
    item_to_package("");
    install_to("Apps.Misc");
    component_flags("None");

    // Reset modified flag as nothing has really changed yet
    _modified = false;

    // Maintain list of all packages
	_next_packager = _first_packager;
	if (_next_packager != 0) _next_packager->_prev_packager = this;
	_prev_packager = 0;
	_first_packager = this;
}

Packager::~Packager()
{
    // Maintain list of all packages
	if (_next_packager) _next_packager->_prev_packager = _prev_packager;
	if (_prev_packager) _prev_packager->_next_packager = _next_packager;
	else _first_packager = _next_packager;
}

/**
 * Get binding for a specified item
 */
PackagerTextEndPoint *Packager::get_binding(PackageItem item)
{
    switch(item)
    {
	   case PACKAGE_NAME: return new PackagerTextEndPoint(this, &Packager::package_name, &Packager::package_name);
	   case VERSION: return new PackagerTextEndPoint(this, &Packager::version, &Packager::version);
	   case PACKAGE_VERSION: return new PackagerTextEndPoint(this, &Packager::package_version, &Packager::package_version);
	   case SECTION: return new PackagerTextEndPoint(this, &Packager::section, &Packager::section);
	   case PRIORITY: return new PackagerTextEndPoint(this, &Packager::priority, &Packager::priority);
       case MAINTAINER: return new PackagerTextEndPoint(this, &Packager::maintainer, &Packager::maintainer);
       case STANDARDS_VERSION: return new PackagerTextEndPoint(this, &Packager::standards_version, &Packager::standards_version);
       case ENVIRONMENT: return new PackagerTextEndPoint(this, &Packager::environment, &Packager::environment);
       case INSTALL_PRIORITY: return new PackagerTextEndPoint(this, &Packager::install_priority, &Packager::install_priority);
       case SUMMARY: return new PackagerTextEndPoint(this, &Packager::summary, &Packager::summary);
       case DESCRIPTION: return new PackagerTextEndPoint(this, &Packager::description, &Packager::description);
	   case HOMEPAGE: return new PackagerTextEndPoint(this, &Packager::homepage, &Packager::homepage); break;
       case LICENCE: return new PackagerTextEndPoint(this, &Packager::licence, &Packager::licence);
       case COPYRIGHT: return new PackagerTextEndPoint(this, &Packager::copyright, &Packager::copyright);

       case ITEM_TO_PACKAGE: return new PackagerTextEndPoint(this, &Packager::item_to_package, &Packager::item_to_package);
       case INSTALL_TO: return new PackagerTextEndPoint(this, &Packager::install_to, &Packager::install_to);

       case DEPENDS: return new PackagerTextEndPoint(this, &Packager::depends, &Packager::depends);
       case RECOMMENDS: return new PackagerTextEndPoint(this, &Packager::recommends, &Packager::recommends);
       case SUGGESTS: return new PackagerTextEndPoint(this, &Packager::suggests, &Packager::suggests);
       case CONFLICTS: return new PackagerTextEndPoint(this, &Packager::conflicts, &Packager::conflicts);
       case OSDEPENDS: return new PackagerTextEndPoint(this, &Packager::osdepends, &Packager::osdepends);

       case COMPONENT_FLAGS: return new PackagerTextEndPoint(this, &Packager::component_flags, &Packager::component_flags);

       case NUM_ITEMS: break; // Dummy value so ignore
    }

    return 0;
}

/**
 * Return next error number or -1 if no more errors
 *
 * Returns to start of list when it gets to the end.
 */
int Packager::next_error(int i) const
{
    if (_error_count == 0) return -1;
    if (i >= NUM_ITEMS) i = NUM_ITEMS -1;

    do
    {
       if (++i == NUM_ITEMS) i = 0;
    } while (_errors[i].empty());

    return i;
}

void Packager::package_name(std::string value)
{
	_package_name = value;
	if (value.empty())
	{
	  set_error(PACKAGE_NAME, "must be entered");
	} else
	  clear_error(PACKAGE_NAME);

	modified(true);
}

void Packager::version(std::string value)
{
	_version = value;
   if (value.empty())
   {
      set_error(VERSION, "must be entered");
   } else
      clear_error(VERSION);

   modified(true);
}


void Packager::package_version(std::string value)
{
   _package_version = value;
   if (value.empty())
   {
      set_error(PACKAGE_VERSION, "must be entered");
   } else
      clear_error(PACKAGE_VERSION);

   modified(true);
}

void Packager::section(std::string value)
{
   _section = value;
   if (value.empty())
   {
      set_error(SECTION, "must be entered");
   } else
      clear_error(SECTION);
   modified(true);
}

void Packager::priority(std::string value)
{
   _priority = value;
   if (value.empty())
   {
      set_error(PRIORITY, "must be entered");
   } else
      clear_error(PRIORITY);
   modified(true);
}

void Packager::maintainer (std::string value)
{
   _maintainer = value;
   if (value.empty())
   {
      set_error(MAINTAINER, "must be entered");
   } else
   {
	   std::string::size_type ltpos = value.find('<');
	   std::string::size_type gtpos = value.find('>');

	   if (ltpos == std::string::npos || gtpos == std::string::npos)
		   set_error(MAINTAINER, "Email address must be included and enclosed in '<' and '>'");
	   else if (ltpos > gtpos)
		   set_error(MAINTAINER, "The '<' must appear before the '>' surrounding the email address");
	   else
		   clear_error(MAINTAINER);
   }

   modified(true);
}

void Packager::standards_version(std::string value)
{
   _standards_version = value;
   if (value.empty())
   {
      set_error(STANDARDS_VERSION, "must be entered");
   } else
   {
	   bool format_ok = true;
	   bool last_dot = true; // Causes required error if first char is a dot.
	   int dot_count = 0;

	   for (std::string::const_iterator i = value.begin();
	        i != value.end() && format_ok; ++i)
	   {
		   if ((*i) == '.')
		   {
			   if (last_dot) format_ok = false;
			   dot_count++;
			   last_dot = true;
		   } else
			   last_dot = false;
	   }

	   if (last_dot) format_ok = false;

	   if (dot_count > 3)
		   set_error(STANDARDS_VERSION, "maximum of 4 components separated by dots ('.')");
	   else if (dot_count < 2)
		   set_error(STANDARDS_VERSION, "must contain at least 3 components separated by dots ('.')");
	   else if (!format_ok)
		   set_error(STANDARDS_VERSION, "must be up to 4 numbers separated by dots ('.')");
	   else
	   {
		   check_standards_version();
	   }
   }

   modified(true);
}


/**
 * Check the given field so see if that it's appropriate
 * for the current standards version
 */
void Packager::check_standards_version()
{
	if (standards_version_lt("0.4.0"))
	{
		if (_component_flags != "None" )
		{
		   set_error(STANDARDS_VERSION, "must be at least 0.4.0 if component flags are set");
		} else
		{
			clear_error(STANDARDS_VERSION);
		}		
	}
	else if (standards_version_lt("0.6.0"))
	{
		if (_environment != "any" && _environment != "arm")
		{
	        set_error(STANDARDS_VERSION, "must be at least 0.6.0 if environment isn't 'any' or 'arm'");
		} else if (!_osdepends.empty())
		{
	   		set_error(STANDARDS_VERSION, "must be at least 0.6.0 if OSDepends is set");
		} else if (!_homepage.empty())
		{
	   		set_error(STANDARDS_VERSION, "must be at least 0.6.0 if Homepage is set");
		} else
		{
			clear_error(STANDARDS_VERSION);
		}		
	} else
	{
		clear_error(STANDARDS_VERSION);
	}	
}

/**
 * Check it the standards version is less than the given value
 *
 * @param value version to check against
 * @returns true if version is less than the given value
 */
bool Packager::standards_version_lt(std::string value)
{
	if (_standards_version.empty()) return true;
	std::string::iterator svi = _standards_version.begin();
	std::string::iterator vi = value.begin();
	int check_val = 0;
	while (svi != _standards_version.end())
	{
		if (*svi == '.')
		{
			svi++;
			int value_part = 0;
			while (vi != value.end() && *vi != '.') value_part = value_part * 10 + (*vi++ - '0');
			if (vi != value.end()) ++vi;
			if (check_val < value_part) return true;
			else if (check_val > value_part) return false;
			check_val = 0;
		} else if (*svi >= '0' && *svi <= '9')
		{
			check_val = check_val * 10 + (*svi++ - '0');
		} else
		{
			// All invalid entries are considered less than
			return true;
		}
	}

	int value_part = 0;
	while (vi != value.end() && *vi != '.') value_part = value_part * 10 + (*vi++ - '0');
	if (vi != value.end()) ++vi;

	if (check_val < value_part) return true;

	// Got to end so its equal
	return false;
}

void Packager::environment(std::string value)
{
   _environment = value;
   check_standards_version();
   if (value.empty())
   {
      set_error(ENVIRONMENT, "must be entered");
   } else
   {
   	 clear_error(ENVIRONMENT);
   }

   modified(true);
}

void Packager::install_priority(std::string value)
{
	_install_priority = value;
	modified(true);
}


void Packager::summary(std::string value)
{
	_summary = value;
	if (value.empty())
	{
	   set_error(SUMMARY, "must be entered");
	} else
	   clear_error(SUMMARY);
	modified(true);
}

void Packager::description(std::string description)
{
	_description = description;
	modified(true);
}

void Packager::homepage(std::string homepage)
{
	_homepage = homepage;
	check_standards_version();
	if (_homepage.empty())
	{
		clear_error(HOMEPAGE);
	} else
	{
		std::string::size_type pos = _homepage.find(':');
		std::string proto;
		if (pos != std::string::npos) proto = tbx::to_lower(_homepage.substr(0, pos));
		if (proto == "http" ||  proto == "https")
		{
			clear_error(HOMEPAGE);
		} else
		{
			set_error(HOMEPAGE, "Protocol 'http' or 'https' must be specified.");
		}
	}

	modified(true);
}

void Packager::licence(std::string licence)
{
	_licence = licence;
	if (licence.empty())
	{
	   set_error(LICENCE, "must be entered");
	} else
	   clear_error(LICENCE);

	modified(true);
}

void Packager::copyright(std::string value)
{
	_copyright = value;

	if (value.empty())
	{
	   set_error(COPYRIGHT, "must be entered");
	} else
	   clear_error(COPYRIGHT);

	modified(true);
}


/**
 * Set item to package.
 *
 * Errors reported via the error listener
 */
void Packager::item_to_package(std::string item)
{
     _item_to_package = item;
     if (item.empty())
     {
         set_error(ITEM_TO_PACKAGE, "must be entered");
     } else
         clear_error(ITEM_TO_PACKAGE);

     modified(true);
}

/**
 * Location to install package to
 *
 * Note: install to is only set to a valid values
 * as it may be used for the save at any time.
 */
void Packager::install_to(std::string where)
{
    if (where.empty())
    {
        set_error(INSTALL_TO, "must be entered");
    } else if (*(where.rbegin()) == '.')
    {
    	set_error(INSTALL_TO, "must not end with a full stop");
    } else
    {
    	std::string::size_type dot_pos = where.find('.');
    	std::string root_dir = (dot_pos == std::string::npos) ? where : where.substr(0, dot_pos);
    	std::string opts;
    	bool bad = true;
    	for (int id = SD_APPS; id <= SD_SYSTEM && bad; id++)
    	{
    		if (!opts.empty()) opts += ",";
    		opts += " ";
    		opts += SpecialDirs[id];
    		if (tbx::equals_ignore_case(root_dir, SpecialDirs[id]))
    		{
    			bad = false;
    		}
    	}

    	if (bad)
    	{
    		set_error(INSTALL_TO, "must start with one of"+opts);
    	} else
    	{
    		bool two_dots = false;
    		while (dot_pos != std::string::npos && !two_dots)
    		{
    			std::string::size_type next_dot = where.find('.', dot_pos+1);
    			if (next_dot == dot_pos+1) two_dots = true;
    			dot_pos = next_dot;
    		}

    		if (two_dots)
    		{
    			set_error(INSTALL_TO, "should not have two dots ('.') together");
    		} else
    		{
    			_install_to = where;
    		    modified(true);
    			clear_error(INSTALL_TO);
    		}
    	}
    }
}

void Packager::component_flags(std::string value)
{
	_component_flags = value;
	check_standards_version();
	 modified(true);
}

void Packager::depends(std::string value)
{
   _depends = value;
   check_depends(DEPENDS, value);
   modified(true);
}

void Packager::recommends(std::string value)
{
   _recommends = value;
   check_depends(RECOMMENDS, value);
   modified(true);
}

void Packager::suggests(std::string value)
{
   _suggests = value;
   check_depends(SUGGESTS, value);
   modified(true);
}

void Packager::conflicts(std::string value)
{
   _conflicts = value;
   check_depends(CONFLICTS, value);
   modified(true);
}

void Packager::osdepends(std::string value)
{
   _osdepends = value;
   check_standards_version();
   check_depends(OSDEPENDS, value);

   modified(true);
}

/**
 * Check dependency format
 */
void Packager::check_depends(PackageItem where, std::string depends)
{
	if (depends.empty())
	{
		clear_error(where);
		return; // Don't need to have a dependency
	}
	std::string err;
	std::string::size_type comma_pos = depends.find(',');
	std::string::size_type start = 0;
	while (comma_pos != std::string::npos && err.empty())
	{
		err = check_one_dependency(depends.substr(start, comma_pos - start));
		start = comma_pos + 1;
		comma_pos = depends.find(',', start);
	}
	if (err.empty()) err = check_one_dependency(depends.substr(start));

	if (err.empty()) clear_error(where);
	else set_error(where, err);
}

std::string Packager::check_one_dependency(std::string dep) const
{
	std::string::const_iterator i = dep.begin();
	std::string err;

	while (i != dep.end() && (*i) == ' ') i++;
	if (i == dep.end())
	{
		err = "empty dependency, have you got too many commas";
	} else if ((*i) == ')')
	{
		err = "Extra ')' in a dependency";
	} else
	{
		while (i != dep.end() && (*i) != '(' && (*i) != ' ') i++;
		while (i != dep.end() && (*i) == ' ') i++;
		if (i != dep.end())
		{
			if (*i != '(')
			{
				err = "dependency package name must end with a comma or a '('";
			} else
			{
				i++;
				while (i != dep.end() && (*i) == ' ') i++;
				if (i == dep.end() || !((*i) == '=' || (*i) == '<' || (*i) == '>'))
				{
					err = "version operator '=', '<<', '>>', '<=' or '>=' missing";
				} else
				{
					if ((*i) == '<')
					{
						i++;
						if (i == dep.end() || ((*i) != '<' && (*i) != '='))
						{
							err = "'<' must be followed by another '<' or and '='";
						}
					} else if ((*i) == '>')
					{
						i++;
						if (i == dep.end() || ((*i) != '>' && (*i) != '='))
						{
							err = "'>' must be followed by another '>' or and '='";
						}
					}
					if (err.empty())
					{
						if (i != dep.end()) i++;
						while (i != dep.end() && (*i) == ' ') i++;
						if (i == dep.end() || (*i) == ')' || (*i) == ',')
							err = "version number missing";
						else if ((*i) == '>' || (*i) == '<' || (*i) == '=')
							err = "extra symbol in version operator";
					}
					if (err.empty())
					{
						while (i != dep.end() && *i != ')' && (*i) != ' ') i++;
						while (i != dep.end() && *i == ' ') i++;
						if (i == dep.end() || *i != ')') err = "missing ')' or a space in the version number";
					}
				}
			}
		}
	}

	return err;
}

/**
 * Add a system variable to the package.
 *
 * Note: will replace an existing variable with the same name
 */
void Packager::add_sysvar(std::string name, std::string value)
{
	_sysvars[name] = value;
	modified(true);
}

/**
 * Get value of a system variable
 */
std::string Packager::sysvar(std::string name) const
{
	SysVarsMap::const_iterator found = _sysvars.find(name);
	if (found == _sysvars.end()) return std::string("");
	else return (*found).second;
}

/**
 * Set the value of a system variable
 *
 * Note: will add the variable if it doesn't already exists
 */
void Packager::sysvar(std::string name, std::string value)
{
	_sysvars[name] = value;
	modified(true);
}

/**
 * Remove a system variable
 */
void Packager::remove_sysvar(std::string name)
{
	_sysvars.erase(name);
	modified(true);
}

/**
 * Clear all system variables
 */
void Packager::clear_sysvars()
{
	if (!_sysvars.empty())
	{
		_sysvars.clear();
		modified(true);
	}
}

/**
 * Add a sprite to the package.
 *
 * Note: will replace an existing spritethe same name
 */
void Packager::add_sprite(std::string name, std::string location)
{
	_sprites[name] = location;
	modified(true);
}

/**
 * Get the location of a sprite
 */
std::string Packager::sprite(std::string name) const
{
	SpritesMap::const_iterator found = _sprites.find(name);
	if (found == _sprites.end()) return std::string("");
	else return (*found).second;
}

/**
 * Set the location of a sprite
 *
 * Note: will add the sprite if it doesn't already exists
 */
void Packager::sprite(std::string name, std::string location)
{
	_sprites[name] = location;
	modified(true);
}

/**
 * Remove a sprite
 */
void Packager::remove_sprite(std::string name)
{
	_sprites.erase(name);
	modified(true);
}

/**
 * Clear all sprites
 */
void Packager::clear_sprites()
{
	if (!_sprites.empty())
	{
		_sprites.clear();
		modified(true);
	}
}

/**
 * Package has been modified
 */
void Packager::modified(bool modified)
{
	if (_modified != modified)
	{
		_modified = modified;
		if (_modified_listener) _modified_listener->modified_changed(modified);
	}
}

/**
 * Set the error message for a package item
 */
void Packager::set_error(PackageItem where, const std::string message)
{
    if (_errors[where].empty()) _error_count++;
    _errors[where] = message;
    if (_listener) _listener->on_item_error(where);
}

/**
 * Clear the error message for a package item
*/
void Packager::clear_error(PackageItem where)
{
    if (!_errors[where].empty())
    {
       _error_count--;
       _errors[where] = "";
       if (_listener) _listener->on_item_valid(where);
    }
}

/**
 * Attempt to load package
 *
 * returns true if this is a package file and it loaded successfully
 *
 * throws PackageFormatException if a package if found but is either invalid
 * or more complicated than this application understands
 */
bool Packager::load(std::string filename)
{
	CZipArchive zip;
	int controlIndex = -1;

	// Temporary test for a zip file as GCCSDK4 rel 1 exception handling is
	// broken. Any other exceptions will still cause a crash

/*
	std::fstream test(filename.c_str(), std::ios_base::in | std::ios_base::out);
	if (!test) return false;
	char temp[2];
	test.read(temp, 2);
	if (temp[0] != 'P' || temp[1] != 'K') return false;
	test.close();

std::cout << "Zip file text succeeded" << std::endl;
*/

	try
	{
		zip.Open(filename.c_str());
		zip.EnableFindFast(true);
		controlIndex = zip.FindFile("RiscPkg/Control");

	} catch(CZipException &e)
	{
		// std::cout << "Not a valid zip file " << e.GetErrorDescription() << std::endl;
		// Not a valid zip file so assume its a file
		// to be packaged
		return false;
	}


	if (controlIndex == -1)
	{
		// No control index so assume it's a zip file
		// to be packaged.
		zip.Close();

		// std::cout << "No Control record in zip file" << std::endl;
		return false;
	}

	// Got here so we have a package to load
	try
	{
		tbx::Hourglass hg();
		_modified = true; // Stop modifed flag from firing an event

		_install_to = ""; // Clear new item default

		// Variable to find item to install
		std::string install_item;
		bool can_grow = true;

		int num_items = zip.GetCount();

		for (int item = 0; item < num_items; item++)
		{
			CZipFileHeader fhead;
			zip.GetFileInfo(fhead, item);

			std::string itemname = fhead.GetFileName();
			std::string base_dir;
			std::string::size_type pos;

			pos = itemname.find('/');
			if (pos == std::string::npos) base_dir = itemname;
			else base_dir = itemname.substr(0,pos);

			SpecialDirId baseDirId = SD_NONE;
			for (int j = 0; j < NumSpecialDirs && baseDirId == SD_NONE; j++)
			{
				if (base_dir == SpecialDirs[j]) baseDirId = (SpecialDirId)j;
			}

			// Don't always get directory entries so check directories
			// for every file.

			switch(baseDirId)
			{
			case SD_APPS:
			case SD_MANUALS:
			case SD_RESOURCES:
			case SD_SYSTEM:
			case SD_TOBELOADED:
			case SD_TOBETASKS:
				// Directories may have a trailing backslash so erase it if thats the case
				if (itemname[itemname.size()-1] == '/') itemname.erase(itemname.size()-1);
				set_install_item(install_item, itemname, can_grow);
				break;

			case SD_NONE:
				{
                    std::string msg("Unsupported install location '");
                    msg += base_dir;
                    msg += "'";
                    throw PackageFormatException(msg);
				}
				break;

			case SD_RISCPKG:
				if (!fhead.IsDirectory())
				{
					if (itemname == "RiscPkg/Control")
					{
						std::string buf;
						if (read_zip_item(zip, item, buf))
						{
							std::istringstream istream(buf);
							read_control(istream);
						}

					} else if (itemname == "RiscPkg/Copyright")
					{
						std::string copytext;
						if (!read_zip_item(zip, item, copytext)) copytext = "";
						copyright(copytext);
					} else
					{
						std::string msg("Unsupported control file '");
						msg += itemname + "'";
						throw PackageFormatException(msg);
					}
				}
				break;

			case SD_SYSVARS:
				{
					std::string name = file_to_var_name(itemname);
					std::string::size_type pos = name.rfind('/');
					if (pos != std::string::npos) name.erase(0, pos+1);
					std::string value;
					read_zip_item(zip, item, value);
					_sysvars[name] = value;
				}
				break;

			case SD_SPRITES:
				{
					std::string name = file_to_var_name(itemname);
					std::string::size_type pos = name.rfind('/');
					if (pos != std::string::npos) name.erase(0, pos+1);
					std::string location;
					read_zip_item(zip, item, location);
					_sprites[name] = location;
				}
				break;
			}
		}

		set_payload(install_item);
		item_to_package("extract " + filename + " " + _item_to_package);

		if (!_sysvars.empty()) sysvars_package_to_obey();
		if (!_sprites.empty()) sprites_package_to_obey();

		// Install to must always be given a value otherwise a save
		// will not work.
		if (_install_to.empty()) install_to("Apps.Misc");

		zip.Close();

	} catch(PackageFormatException)
	{
		zip.Close(CZipArchive::afAfterException);
		throw;
	} catch(std::bad_alloc)
	{
		zip.Close(CZipArchive::afAfterException);
		throw PackageFormatException("Unable to allocate enough memory to load package");
	} catch(...)
	{
		zip.Close(CZipArchive::afAfterException);
		throw PackageFormatException("Unexpected exception thrown during load package");
	}

	_modified = false; // No modifications immediately after load

	return true;
}


/**
 * Read items from control record.
 *
 * This is based on read routine from LibPkg
 *
 * Throws an PackageFormatException if there is a syntax error or
 * a field type this program doesn't understand
 */
void Packager::read_control(std::istream &in)
{
	std::string name, value;
	bool done=false;

	while (in&&!done)
	{
		// Get line from input stream.
		std::string line;
		getline(in,line);

		// Convert to sequence.
		std::string::const_iterator first=line.begin();
		std::string::const_iterator last=line.end();

		// Strip trailing spaces.
		while ((last!=first)&&isspace(*(last-1))) --last;


		if ((first==last)||isspace(*first))
		{
			// Line is blank or begins with a space:
			// Skip leading spaces.
			std::string::const_iterator p=first;
			while ((p!=last)&&isspace(*p)) ++p;
			if (p==last)
			{
				// Line is blank (or contains only spaces):
				done=true;
			}
			else
			{
				// Line is a continuation line:
				// Check whether there is a field to continue.
				if (name.empty())
					throw PackageFormatException(
						"Continuation line not allowed here in RiscPkg/Control");

				// If line contains nothing but a period
				// then skip that character.
				if ((p+1==last)&&(*p=='.')) ++p;

				// Append continuation line to field.
				value+=std::string("\n");
				value+=std::string(p,last);
			}
		}
		else
		{
			if (!name.empty()) set_control_field(name, value);

			// Line does not begin with a space:
			// Parse fieldname.
			std::string::const_iterator p=first;
			while ((p!=last)&&(*p!=':'))
			{
				if (isspace(*p))
					throw PackageFormatException("Syntax error in RiscPkg/Control");
				++p;
			}
			name=std::string(first,p);

			// Parse colon at end of fieldname.
			if ((p!=last)&&(*p==':')) ++p;
			else throw PackageFormatException("':' expected in RiscPkg/Control");

			// Skip spaces.
			while ((p!=last)&&isspace(*p)) ++p;

			// Set beginning of value
			value = std::string(p, last);

		}
	}

	if (!name.empty()) set_control_field(name, value);
}

/**
 * Set control field with it's value.
 *
 * throws PackageFormatException if this program doesn't understand the field name.
 */
void Packager::set_control_field(std::string name, std::string value)
{
	if (name.compare("Package") == 0)
	{
		package_name(value);
	} else if (name.compare("Version") == 0)
	{
		std::string::size_type rpos = value.rfind('-');
		if (rpos == std::string::npos)
			throw PackageFormatException("Version must end with package version after a '-'");

		version(value.substr(0, rpos));
		package_version(value.substr(rpos+1));
	} else if (name.compare("Section") == 0)
	{
		section(value);
	} else if (name.compare("Priority") == 0)
	{
		priority(value);
	} else if (name.compare("Maintainer") == 0)
	{
		maintainer(value);
	} else if (name.compare("Standards-Version") == 0)
	{
		standards_version(value);
	} else if (name.compare("Description") == 0)
	{
		std::string::size_type eolpos = value.find('\n');
		if (eolpos == std::string::npos)
		{
			summary(value);
		} else
		{
			summary(value.substr(0, eolpos));
			description(value.substr(eolpos+1));
		}
	} else if (name.compare("Homepage") == 0)
	{
		homepage(value);
	} else if (name.compare("Licence") == 0)
	{
		licence(value);
	} else if (name.compare("Depends") == 0)
	{
	    depends(value);
	} else if (name.compare("Recommends") == 0)
	{
	    recommends(value);
	} else if (name.compare("Suggests") == 0)
	{
	   suggests(value);
	} else if (name.compare("Conflicts") == 0)
	{
	   conflicts(value);
	} else if (name.compare("Components") == 0)
	{
		if (value.empty()) component_flags("None");
		else
		{
			// Ensure flags are in same order as needed in stringset
			std::string::size_type bracket_pos = value.find('(');;
			if (bracket_pos == std::string::npos) component_flags("None");
			else
			{
				std::string ui_value;
				if (value.find("Movable", bracket_pos) != std::string::npos) ui_value = "Movable";
				if (value.find("LookAt", bracket_pos) != std::string::npos)
				{
					if (!ui_value.empty()) ui_value += " ";
					ui_value += "LookAt";
				}
				component_flags(ui_value);
			}
		}
	} else if (name.compare("Environment") == 0)
	{
	   environment(value);
	} else if (name.compare("InstallPriority") == 0)
	{
	   install_priority(value);
	} else if (name.compare("OSDepends") == 0)
	{
	   osdepends(value);
	} else
	{
		throw PackageFormatException("Unable to process field '" + name + "' in RiscPkg/Control");
	}
}

/**
 * Compare the passed path with the item name and update it to be the
 * item that is being installed.
 *
 * @param install_item item name to update
 * @param item_name name of item to compare with current install_item
 * @param can_grow starts as true and is changed to false when the install_item
 *        is stopped from getting any larger.
 */
void Packager::set_install_item(std::string &install_item, const std::string &item_name, bool &can_grow)
{
	std::string::size_type install_item_size = install_item.size();
	std::string::size_type item_size = item_name.size();
	std::string::size_type match_pos;

	match_pos = std::string::npos;
	if (install_item_size == 0)
	{
		install_item = item_name;
	}
	else if (item_size <= install_item_size)
	{
		if (item_name == install_item.substr(0, item_size))
		{
			if (item_size != install_item.size()
				&& install_item[item_size] != '/')
			{
				can_grow = false;
				std::string::size_type pos = item_name.rfind('/');
				install_item = item_name.substr(0, pos);
			}
		} else
		{
			match_pos = item_name.rfind('/');
		}
	} else if (can_grow)
	{
		if (item_name.substr(0, install_item_size) == install_item)
		{
			if (item_name[install_item_size] == '/')
			{
				install_item = item_name;
			} else
			{
				match_pos = install_item.rfind('/');
			}
		} else
		{
			match_pos = install_item.rfind('/');
		}
	}

	if (match_pos != std::string::npos)
	{
		while (match_pos != std::string::npos
				&& item_name.substr(0, match_pos) != install_item.substr(0, match_pos)
			   )
		{
			match_pos = item_name.rfind('/',match_pos-1);
		}

		// Should at least match at base dir level
		if (match_pos == std::string::npos)
		{
			std::string msg;
			msg = "Only Install of one item (file or folder) supported. Found '";
			msg += install_item;
			msg += "' and '" + item_name + "'";
			throw PackageFormatException(msg);
		} else
		{
			install_item = item_name.substr(0, match_pos);
			can_grow = false;
		}
	}
}

/**
 * Set the install to and install package from an item name from
 * the zip file.
 */
void Packager::set_payload(const std::string &name)
{
	std::string roname = zip_to_riscos_name(name);
	std::string::size_type leaf_pos = roname.rfind('.');
	item_to_package(roname);

	if (leaf_pos == std::string::npos)
	{
		install_to("");
	} else
	{
		install_to(roname.substr(0, leaf_pos));
	}
}

/**
 * Convert the <Package$@...> to <Obey$Dir>
 */
void Packager::sysvars_package_to_obey()
{
	SysVarsMap::iterator i;
	std::string package_dir("<Packages$@");
	package_dir += _install_to;
	std::string::size_type index = _item_to_package.rfind('.');
	if (index != std::string::npos) package_dir += _item_to_package.substr(index);
	package_dir += '>';

	for (i = _sysvars.begin(); i != _sysvars.end(); ++i)
	{
		std::string value = i->second;
		index = 0;
		while ((index = tbx::find_ignore_case(value, package_dir, index))!= std::string::npos)
		{
			value.replace(index, package_dir.length(), "<Obey$Dir>");
			index++;
		}
		i->second = value;
	}
}

/**
 * Convert the <Package$@...> to <Obey$Dir> for sprite locations
 */
void Packager::sprites_package_to_obey()
{
	SpritesMap::iterator i;
	std::string package_dir("<Packages$@");
	package_dir += _install_to;
	std::string::size_type index = _item_to_package.rfind('.');
	if (index != std::string::npos) package_dir += _item_to_package.substr(index);
	package_dir += '>';

	for (i = _sprites.begin(); i != _sprites.end(); ++i)
	{
		std::string value = i->second;
		index = 0;
		while ((index = tbx::find_ignore_case(value, package_dir, index))!= std::string::npos)
		{
			value.replace(index, package_dir.length(), "<Obey$Dir>");
			index++;
		}
		i->second = value;
	}
}

/**
 * Convert filename from within a zip to a RISC OS filename
 */
std::string Packager::zip_to_riscos_name(const std::string &zipname) const
{
	std::string roname(zipname);
	std::string::size_type pos = 0;
	while ((pos = roname.find_first_of("./", pos + 1)) != std::string::npos)
	{
		if (roname[pos] == '.') roname[pos] = '/';
		else roname[pos] = '.';
	}

	return roname;
}

/**
 * Convert a RISC OS filename  to filename within a zip
 */
std::string Packager::riscos_to_zip_name(const std::string &riscosname) const
{
	// Currently the transformation is identical, but it
	// may change in future.
	return zip_to_riscos_name(riscosname);
}


/**
 * Save packager
 *
 * returns true if successful
 */
bool Packager::save(std::string filename)
{
	CZipArchive zip;
	bool ok = false;

	try
	{
		tbx::Hourglass hg;

		std::string oldzipfile, oldzipitem;
		bool delete_old_zip = false;
		bool files_in_zip = copying_from_zip(filename, oldzipfile, oldzipitem, delete_old_zip);

		zip.Open(filename.c_str(), CZipArchive::zipCreate);

		// Must have an install to location
		if (_install_to.empty()) install_to("Apps.Misc");

		write_control(zip);
		write_copyright(zip);
		write_sysvars(zip);
		write_sprites(zip);

		hg.percentage(1);

		if (files_in_zip)
		{
			copy_zip_files(zip, oldzipfile, oldzipitem);
			if (delete_old_zip) tbx::Path(oldzipfile).remove();
		} else
		{
			tbx::Path files(_item_to_package);

			// Set size of base file name so it can be removed from zip filenames
			_base_dir_size = files.parent().name().length() + 1;
			std::vector<std::pair<tbx::Path, tbx::PathInfo> > file_list;

			tbx::PathInfo root_info;
			if (!files.path_info(root_info))
			{
				std::string msg("Unable to read file/directory ");
				msg += _item_to_package;
				throw PackageCreateException(msg);
			}

			if (root_info.directory())
			{
				get_file_list(files, file_list);
			} else
			{
			    if (root_info.image_file())
			    {
			       // Image file systems don't by default give a file type
			       // so re-read it and calculate
			       files.raw_path_info(root_info, true);
			    }
				file_list.push_back(std::pair<tbx::Path, tbx::PathInfo>(files, root_info));
			}

			hg.percentage(2);
			unsigned int last_percent = 2;
			unsigned int total_files = file_list.size();

			for (unsigned int i = 0; i != total_files; i++)
			{
				copy_file(zip, file_list[i].first, file_list[i].second);
				unsigned int pc = i * 100 / total_files;
				if (pc > last_percent)
				{
					last_percent = pc;
					hg.percentage(pc);
				}
			}
		}

		hg.percentage(99);
		zip.Close();

	    ok = true;

	} catch(CZipException &e)
	{
		std::string errmsg("Failed to create zip file: ");
		std::string desc =e.GetErrorDescription();
		errmsg += desc;
		tbx::report_error(errmsg.c_str());
	} catch(PackageCreateException &e)
	{
		tbx::report_error(e.what().c_str());
	} catch(std::bad_alloc)
	{
		tbx::report_error("Unable to allocate enough memory to create package");
	} catch(...)
	{
		tbx::report_error("Unexpected exception thrown during package creation");
	}

	return ok;
}

/**
 * Write control record to given stream
 */
void Packager::write_control(CZipArchive &zip) const
{
	std::ostringstream os;

	if (!_package_name.empty())
	    os << "Package: " << _package_name << std::endl;
	if (!_version.empty())
	    os << "Version: " << _version << "-" << _package_version << std::endl;
	if (!_section.empty())
	    os << "Section: " << _section << std::endl;
	if (!_priority.empty())
	     os << "Priority: " << _priority << std::endl;
	if (!_maintainer.empty())
	   os << "Maintainer: " << _maintainer << std::endl;
	if (!_standards_version.empty())
	   os << "Standards-Version: " << _standards_version << std::endl;
	if (!_environment.empty())
		   os << "Environment: " << _environment << std::endl;
	if (!_install_priority.empty() && _install_priority != "0")
		   os << "InstallPriority: " << _install_priority << std::endl;

	if (!_licence.empty())
	   os << "Licence: " << _licence << std::endl;
	if (!_summary.empty())
		os << "Description: " << _summary << std::endl;
	if (!_description.empty())
	{
	    if (_summary.empty()) os << "Description: ";
  		std::string::size_type solpos = 0, eolpos, wspos;
  		int blank_line = 0;
		while (solpos < _description.size()
			&& (eolpos = _description.find('\n', solpos))!=std::string::npos
			)
		{
			wspos = solpos;
			while (wspos < eolpos && _description[wspos] == ' ') wspos++;
			if (eolpos == wspos) blank_line++;
			else
			{
				while (blank_line > 0) { blank_line--; os << " ." << std::endl;}
			    os << " " << _description.substr(solpos, eolpos - solpos) << std::endl;
			}
			solpos = eolpos+1;
		}
		if (solpos < _description.size())
		{
			while (blank_line > 0) { blank_line--; os << " ." << std::endl;}
			os << " "  << _description.substr(solpos) << std::endl;
		}
	}
	if (!_homepage.empty())
		os << "Homepage: " << _homepage << std::endl;

	if (_component_flags != "None")
	{
		std::string::size_type leaf_pos = _item_to_package.rfind('.');
		std::string leaf_name = (leaf_pos == std::string::npos) ? _item_to_package : _item_to_package.substr(leaf_pos+1);
		os << "Components: " << _install_to << "." << leaf_name << " (" << _component_flags << ")" << std::endl;
	}

    if (!_depends.empty())
      os << "Depends: " << _depends << std::endl;
    if (!_recommends.empty())
      os << "Recommends: " << _recommends << std::endl;
    if (!_suggests.empty())
      os << "Suggests: " << _suggests << std::endl;
    if (!_conflicts.empty())
      os << "Conflicts: " << _conflicts << std::endl;
    if (!_osdepends.empty())
      os << "OSDepends: " << _osdepends << std::endl;

	write_text_file(zip, "RiscPkg/Control", os.str());
}


/**
 * Write the copyright file
 */
void Packager::write_copyright(CZipArchive &zip) const
{
	write_text_file(zip, "RiscPkg/Copyright", _copyright.c_str());
}

/**
 * Write system variables to the zip file
 */
void Packager::write_sysvars(CZipArchive &zip) const
{
	SysVarsMap::const_iterator i;
	std::string obey_dir("<Packages$@");
	obey_dir += _install_to;
	std::string::size_type index = _item_to_package.find_last_of("./");
	if (index != std::string::npos && index != _item_to_package.length()-1)
		obey_dir += "." + _item_to_package.substr(index+1);
	obey_dir += '>';

	for (i = _sysvars.begin(); i != _sysvars.end(); ++i)
	{
		std::string filename = "SysVars/" + var_to_file_name((*i).first);
		std::string value = (*i).second;
		std::string::size_type pos = 0;
		while ((pos = tbx::find_ignore_case(value, "<obey$dir>", pos)) != std::string::npos)
		{
			value.replace(pos, 10, obey_dir);
			pos++;
		}

		write_text_file(zip, filename.c_str(), value);
	}
}

/**
 * Write the sprites to the file
 */
void Packager::write_sprites(CZipArchive &zip) const
{
	SpritesMap::const_iterator i;
	std::string obey_dir("<Packages$@");
	obey_dir += _install_to;
	std::string::size_type index = _item_to_package.rfind('.');
	if (index != std::string::npos) obey_dir += _item_to_package.substr(index);
	obey_dir += '>';

	for (i = _sprites.begin(); i != _sprites.end(); ++i)
	{
		std::string filename = "Sprites/" + var_to_file_name((*i).first);
		std::string value = (*i).second;
		std::string::size_type pos = 0;
		while ((pos = tbx::find_ignore_case(value, "<obey$dir>", pos)) != std::string::npos)
		{
			value.replace(pos, 10, obey_dir);
			pos++;
		}

		write_text_file(zip, filename.c_str(), value);
	}
}

/**
 * Write a text file with the given text to the zip file
 */
void Packager::write_text_file(CZipArchive &zip, const char *filename, std::string text) const
{
	CZipFileHeader fhead;
	fhead.SetFileName(filename);
	fhead.SetModificationTime(time(NULL));

	RISCOSZipExtra textextra(0xFFF);

    // Local entry
	CZipExtraData *extra = fhead.m_aLocalExtraData.CreateNew(textextra.tag());
    extra->m_data.Allocate(textextra.size());
	memcpy(extra->m_data, textextra.buffer(), textextra.size());
	// Central directory entry
	extra = fhead.m_aCentralExtraData.CreateNew(textextra.tag());
    extra->m_data.Allocate(textextra.size());
	memcpy(extra->m_data, textextra.buffer(), textextra.size());

	zip.OpenNewFile(fhead);
	zip.WriteNewFile(text.c_str(), text.size());
	zip.CloseNewFile();
}

/**
 * Get list of files to copy
 *
 * Recurses down directories
 */
void Packager::get_file_list(const tbx::Path &dirname, std::vector<std::pair<tbx::Path, tbx::PathInfo> > &file_list) const
{
	std::vector<std::string> dirnames;

	for (tbx::PathInfo::Iterator i = tbx::PathInfo::begin(dirname, "*");
	        i != tbx::PathInfo::end(); ++i)
	{
		tbx::PathInfo entry(*i);

		if (entry.directory())
		{
			// Go down directories after processing all files
			dirnames.push_back(entry.name());
		} else
		{
			tbx::Path filename(dirname , entry.name());
			file_list.push_back(std::pair<tbx::Path, tbx::PathInfo>(filename, entry));
		}
	}

	std::vector<std::string>::iterator i;
	for (i = dirnames.begin(); i != dirnames.end(); ++i)
	{
		tbx::Path subdirname(dirname, (*i));
		get_file_list(subdirname, file_list);
	}
}

/**
 * Copy files from specified directory to zip file.
 */
void Packager::copy_files(CZipArchive &zip, const tbx::Path &dirname) const
{
	std::vector<std::string> dirnames;

	for (tbx::PathInfo::Iterator i = tbx::PathInfo::begin(dirname, "*");
	        i != tbx::PathInfo::end(); ++i)
	{
		tbx::PathInfo entry(*i);
		if (entry.directory())
		{
			// Go down directories after processing all files
			dirnames.push_back(entry.name());
		} else
		{
			tbx::Path filename(dirname , entry.name());
			copy_file(zip, filename, entry);
		}
	}

	std::vector<std::string>::iterator i;
	for (i = dirnames.begin(); i != dirnames.end(); ++i)
	{
		tbx::Path subdirname(dirname, (*i));
		copy_files(zip, subdirname);
	}
}

/**
 * Copy a single file to the archive
 */
void Packager::copy_file(CZipArchive &zip, const tbx::Path &filename) const
{
	tbx::PathInfo entry;
	filename.path_info(entry);

	copy_file(zip, filename, entry);
}

/**
 * Copy a single file and its attribute to the archive
 */
void Packager::copy_file(CZipArchive &zip, const tbx::Path &filename, tbx::PathInfo &entry) const
{
	// swap "." and slashes in name to put in zip file
	std::string nameinzip = _install_to + "." + filename.name().substr(_base_dir_size);
	nameinzip = riscos_to_zip_name(nameinzip);

	CZipFileHeader fhead;
	fhead.SetFileName(nameinzip.c_str());

	if (entry.has_file_type())
	{
		long long csecs_since_1900 = entry.modified_time().centiseconds();
		long long secs_between = 25567; // days
		secs_between *= 24 * 60 * 60; // seconds
		time_t secs_since_1970 = (time_t)(csecs_since_1900/100 - secs_between);

		fhead.SetModificationTime(secs_since_1970);
	} else
	{
	    fhead.SetModificationTime(time(NULL));
	}

	RISCOSZipExtra extra(entry);

    // Local filetype extra data
	CZipExtraData *extra_data = fhead.m_aLocalExtraData.CreateNew(extra.tag());
    extra_data->m_data.Allocate(extra.size());
	memcpy(extra_data->m_data, extra.buffer(), extra.size());
	// Central Directory filetype extra data
	extra_data = fhead.m_aCentralExtraData.CreateNew(extra.tag());
    extra_data->m_data.Allocate(extra.size());
	memcpy(extra_data->m_data, extra.buffer(), extra.size());

	zip.OpenNewFile(fhead);

	/* Copy file data */
	int filesize = entry.length();
	if (filesize > 0)
	{
		std::ifstream from_file(filename.name().c_str());
		while (filesize > copy_buffer_size)
		{
			from_file.read(copy_buffer, copy_buffer_size);
			zip.WriteNewFile(copy_buffer, copy_buffer_size);
			filesize -= copy_buffer_size;
		}
		if (filesize > 0)
		{
			from_file.read(copy_buffer, filesize);
			zip.WriteNewFile(copy_buffer, filesize);
		}
	}

	zip.CloseNewFile();
}


/**
 * Check if the files to install are in a zip file.
 *
 * Will also return false if item to package format doesn't include
 * the zipfile and zipitem.
 *
 * @param zipfile update to zipfile name if files are in a zip
 * @param zipitem item in zipfile if files are in a zip
 * @returns true if files are in a zip file
 */
bool Packager::files_in_zip(std::string &zipfile, std::string &zipitem) const
{
	if (_item_to_package.substr(0, 8) != "extract ") return false;

	zipfile = _item_to_package.substr(8);
	std::string::size_type space_pos = zipfile.find(' ');
	if (space_pos != std::string::npos)
	{
	   zipitem = riscos_to_zip_name(zipfile.substr(space_pos+1));
	   zipfile = zipfile.substr(0, space_pos);
	} else
	{
		zipitem.clear();
	}

	return !(zipitem.empty() || zipfile.empty());
}

/**
 * Read item from zip file into a string.
 *
 * @param zip CZipArchive to read item from
 * @param index index of item to read
 * @param item string to be updated with data extracted from zipfile
 *
 * @returns true if successful and data is updated
 */
bool Packager::read_zip_item(CZipArchive &zip, int index, std::string &item)
{
	CZipMemFile mf;
	if (zip.ExtractFile(index, mf))
	{
		int len = mf.GetLength();

		char *data = (char *)mf.Detach();
		item.assign(data, len);
		free(data);

		return true;
	}

	return false;
}

/**
 * Read zip item to a string.
 *
 * @param zipfile to read
 * @param zipname item to read in zip file (case insensitive)
 * @param resulting data
 * @returns true if data is read
 */
bool Packager::read_zip_item(const std::string &zipfile, const std::string &zipname, std::string &data)
{
	CZipArchive zip;

	try
	{
		zip.Open(zipfile.c_str());
		zip.EnableFindFast(true);
		int index = zip.FindFile(zipname.c_str(), CZipArchive::ffNoCaseSens);
		if (index == ZIP_FILE_INDEX_NOT_FOUND) return false;

		return read_zip_item(zip, index, data);

	} catch(CZipException &e)
	{
		// std::cout << "Not a valid zip file " << e.GetErrorDescription() << std::endl;
		// Not a valid zip file so assume its a file
		// to be packaged
		return false;
	}
}


/**
 * Check if we are extracting files to copy from an existing zip file
 *
 * If the zipfile has the same name as the one we are going to
 * create rename it so it can be copied from.
 *
 * @param filename name of new package file (to check if old needs renaming)
 * @param oldzipname updated to name of the zip file to extract files from
 * @param oldzipitem updated to name of file or directory to be extracted
 * @param delete_old_zip set to true if oldzipname should be deleted after use
 * @returns true if files are being copied from a zip file
 */
bool Packager::copying_from_zip(const std::string &filename, std::string &oldzipname, std::string &oldzipitem, bool &delete_old_zip)
{
	delete_old_zip = false;
	if (_item_to_package.substr(0, 8) != "extract ") return false;

	std::string tempfilename;
    if (!files_in_zip(oldzipname, oldzipitem))
    {
       throw PackageCreateException("'extract' install item does not include a filename and/or item within it");
    }

    if (tbx::equals_ignore_case(oldzipname, filename))
    {
        int check_no = 0;

        // Rename old zip file, so we can extract the files
        // from it.
        do
        {
           check_no++;
           tempfilename = filename + "bak" + tbx::to_string(check_no);
        } while (tbx::Path(tempfilename).exists() && check_no < 500);

        if (check_no < 500)
		{
			try
			{
			    tbx::Path(filename).rename(tempfilename);
			} catch(...)
			{
				check_no = 500; // * Force failure
			}
		}
        if (check_no == 500)
        {
           std::string msg("Unable to backup old package/zip file ");
           msg += filename;
           throw PackageCreateException(msg);
        }

        oldzipname = tempfilename;

        delete_old_zip = true;
    }

    return true;
}

/**
 * Copy files from zip archive
 */
void Packager::copy_zip_files(CZipArchive &zip, const std::string &zipfilename, const std::string &zipitem)
{
   CZipArchive oldzip;
   tbx::Hourglass hg;

   hg.percentage(1);

   oldzip.Open(zipfilename.c_str(), CZipArchive::zipOpenReadOnly);

   CZipWordArray indices;
   std::string pattern(zipitem);
   std::string::size_type esc_pos = pattern.find_first_of("*?^-\[]");
   while (esc_pos != std::string::npos)
   {
      pattern.insert(esc_pos, 1, '\\');
      esc_pos = pattern.find_first_of("*?^-\[]", esc_pos+2);
   }
   if (*zipitem.rbegin() != '/') pattern += "/";
   pattern += "*";

   oldzip.FindMatches(pattern.c_str(), indices, true);

   hg.percentage(2);

   unsigned int total_files = indices.GetSize();

   if (total_files == 0)
   {
       // May be a single file so try again without directory wildcard
       pattern.erase(pattern.size()-2,2);
       oldzip.FindMatches(pattern.c_str(), indices, true);
       total_files = indices.GetSize();
   }

   if (total_files == 0)
   {
	  oldzip.Close();
      throw PackageCreateException("Failed to find files in old zip file");
   }

   std::string newloc = riscos_to_zip_name(_install_to);
   std::string::size_type spos = zipitem.rfind('/');
   if (spos == std::string::npos)
   {
	  oldzip.Close();
      throw PackageCreateException("Unable to determine location of old zip file");
   }
   std::string oldloc = zipitem.substr(0, spos);

   unsigned int last_percent = 2;
   unsigned int pc;

   if (tbx::equals_ignore_case(newloc, oldloc))
   {
	   // Location unchanged so do a straight copy
	   // zip.GetFromArchive(oldzip, indices) doesn't work here
	   // so copy files individually
	   for (unsigned int i = 0; i < total_files;i++)
	   {
		   if (!zip.GetFromArchive(oldzip, indices[i]))
		   {
			  oldzip.Close();
			  throw PackageCreateException("Failed to copy file from old archive");
		   }
		   pc = 100 * i / total_files;
		   if (pc > last_percent)
		   {
			   last_percent = pc;
			   hg.percentage(pc);
		   }
	   }
   } else
   {
	   // Location changed so copy to a new location
	   for (unsigned int i = 0; i < total_files;i++)
	   {
          CZipFileHeader fhead;
          oldzip.GetFileInfo(fhead, indices[i]);

	      std::string name = fhead.GetFileName();
	      name = newloc + name.substr(oldloc.size());
		  if (!zip.GetFromArchive(oldzip, indices[i], name.c_str()))
		  {
			  oldzip.Close();
			  throw PackageCreateException("Failed to copy file from old archive");
		  }

		   pc = 100 * i / total_files;
		   if (pc > last_percent)
		   {
			   last_percent = pc;
			   hg.percentage(pc);
		   }
	   }
   }

   oldzip.Close();
}

/**
 * Convert sysvars or sprites variable name to a filename
 */

std::string Packager::var_to_file_name(const std::string &varname) const
{
	std::string result;
	std::string::const_iterator i;
	for (i = varname.begin(); i != varname.end(); ++i)
	{
		switch(*i)
		{
		case '.': result += "=2E"; break;
		case ':': result += "=3A"; break;
		case '$': result += "=24"; break;
		case '&': result += "=26"; break;
		case '@': result += "=40"; break;
		case '^': result += "=5E"; break;
		case '%': result += "=25"; break;
		case '\\': result += "=5C"; break;
		case '=': result += "=3D"; break;
		default:
			result += *i;
			break;
		}
	}

	return result;
}

/**
 * Convert filename for sysvars and sprites to variable name
 */
std::string Packager::file_to_var_name(const std::string &filename) const
{
	std::string result;
	std::string::size_type pos, last_pos = 0, size = filename.size();
	while (last_pos < size && (pos = filename.find('=', last_pos)) != std::string::npos)
	{
		if (pos > last_pos) result += filename.substr(last_pos, pos - last_pos);
		if (pos >= size - 3)
		{
			result += filename[pos];
			last_pos = pos+1;
		} else
		{
			char *dum;
			char ch = (char)strtoul(filename.substr(pos+1,2).c_str(), &dum, 16);
			if (ch) result += ch;
			else result += filename.substr(pos,3);
			last_pos = pos + 3;
		}
	}
	if (last_pos < size) result += filename.substr(last_pos, size-last_pos);
	return result;
}


