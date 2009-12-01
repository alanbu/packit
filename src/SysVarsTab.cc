/*********************************************************************
* Copyright 2009 Alan Buckley
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

/*
 * SysVarsTab.cc
 *
 *  Created on: 22-May-2009
 *      Author: alanb
 */

#include "SysVarsTab.h"
#include "eventids.h"
#include "EditVarDlg.h"

#include <fstream>
#include <string.h>

#include "tbx/path.h"
#include "tbx/stringutils.h"

SysVarsTab::SysVarsTab(tbx::Window window, Packager &packager) :
	_packager(packager),
	_window(window),
	_add_var_command(this, &SysVarsTab::add_var),
    _edit_var_command(this, &SysVarsTab::edit_var),
    _remove_var_command(this, &SysVarsTab::remove_var),
    _clear_vars_command(this, &SysVarsTab::clear_vars),
    _scan_boot_command(this, &SysVarsTab::scan_boot)
{
	_var_list = window.gadget(0);

	window.add_command(EID_SYSVARS_ADD, &_add_var_command);
	window.add_command(EID_SYSVARS_EDIT, &_edit_var_command);
	window.add_command(EID_SYSVARS_REMOVE, &_remove_var_command);
	window.add_command(EID_SYSVARS_CLEAR, &_clear_vars_command);
	window.add_command(EID_SYSVARS_SCAN_BOOT, &_scan_boot_command);

	_edit_var_dlg = 0;
}

SysVarsTab::~SysVarsTab()
{
	delete _edit_var_dlg;
}

void SysVarsTab::package_loaded()
{
	Packager::SysVarsMap::const_iterator i;

	for (i = _packager.sysvars().begin(); i != _packager.sysvars().end(); ++i)
	{
		set_var((*i).first.c_str(), (*i).second.c_str());
	}
}

/**
 * Return index for a name
 */
int SysVarsTab::name_index(const std::string &name) const
{
	std::string map_name(tbx::to_lower(name));
	std::map<std::string, int>::const_iterator found = _var_to_index.find(map_name);
	return (found != _var_to_index.end()) ? found->second : -1;
}

/**
 * Add a new system variable
 */
void SysVarsTab::add_var()
{
	if (_edit_var_dlg == 0) _edit_var_dlg = new EditVarDlg(this);
	_edit_var_dlg->adding();
	printf("pre show\n");
	_edit_var_dlg->show(_window);
	printf("post show\n");
}

/**
 * Edit a system variable
 */
void SysVarsTab::edit_var()
{
	int index = _var_list.first_selected();
	if (index >= 0)
	{
		std::string name = _var_list.item_text(index);
		std::string value;
		std::string::size_type space_pos = name.find(' ');
		if (space_pos != std::string::npos)
		{
			value = name.substr(space_pos+1);
			name.erase(space_pos);
		}

		if (_edit_var_dlg == 0) _edit_var_dlg = new EditVarDlg(this);
		_edit_var_dlg->editing(index, name, value);
		_edit_var_dlg->show(_window);
	}
}

/**
 * Remove a system variable
 */
void SysVarsTab::remove_var()
{
	int index = _var_list.first_selected();
	if (index >= 0)
	{
		_var_list.delete_item(index);

		std::map<std::string, int>::iterator i, found = _var_to_index.end();
		for (i = _var_to_index.begin(); i != _var_to_index.end(); ++i)
		{
			if ((*i).second == index) found = i;
			else if ((*i).second > index)
			{
				(*i).second -= 1;
			}
		}
		if (found != _var_to_index.end()) _var_to_index.erase(found);
	}
}

/**
 * Delete a system variable
 */
void SysVarsTab::clear_vars()
{
	_var_list.clear();
	_var_to_index.clear();
	_packager.clear_sysvars();
}

/**
 * Scan the !Boot file for a system variable
 */
void SysVarsTab::scan_boot()
{
	std::string zipfile, zipitem;
	if (_packager.files_in_zip(zipfile, zipitem))
	{
		zipitem += "/!Boot";
		std::string buf;
		if (_packager.read_zip_item(zipfile, zipitem, buf))
		{
			std::istringstream is(buf);
			scan_boot(is);
		}
	} else
	{
		tbx::Path path(_packager.item_to_package(), "!Boot");
		if (path.exists())
		{
			std::ifstream is(path);
			if (is) scan_boot(is);
		}
	}
}

/**
 * Scan stream opened on a boot file
 */
void SysVarsTab::scan_boot(std::istream &is)
{
	char line[512];
	char value[512];

	while (is.getline(line, sizeof(line)))
	{
		char *p = line;
		while (*p == ' ') p++;
		if (strnicmp(p, "set ",4) == 0)
		{
			char *var_name;

			p+= 4;

			while (*p == ' ') p++;
			var_name = p;

			while (*p && *p != ' ') p++;
			if (*p)
			{
				char *vp = value;
				*p++ = 0;
				while (*p == ' ') p++;

				while (*p)
				{
					if (*p == '<')
					{
						char *lookup = ++p;
						while (*p && *p != '>') p++;
						if (*p)
						{
							*p++ = 0;
							std::string lval=_packager.sysvar(lookup);
							if (lval.empty())
							{
								*vp++='<';
								strcpy(vp, lookup);
								vp = vp + strlen(vp);
								*vp++='>';
							} else
							{
								lval.copy(vp, lval.length());
								vp += lval.length();
							}
						}
					}
					else *vp++ = *p++;
				}
				*vp = 0;
				set_var(var_name, value);
			}
		}
	}
}

/**
 * Set new value or add a variable to the scroll list
 */
void SysVarsTab::set_var(const char *var_name, const char *value)
{
	_packager.sysvar(var_name, value);

	std::string name;
	const char *p = var_name;
	while (*p) name += std::tolower(*p++);

	std::string display(var_name);
	display += " ";
	display += value;

	std::map<std::string, int>::iterator found = _var_to_index.find(name);
	if (found != _var_to_index.end())
	{
		_var_list.item_text((*found).second, display);
	} else
	{
		int index= _var_to_index.size();
		_var_list.add_item(display);
		_var_to_index[name] = index;
	}
}

/**
 * Replace a variable that has been modified
 */
void SysVarsTab::replace_var(int index, const std::string &name, const std::string &value)
{
	std::string old_name = _var_list.item_text(index);
	std::string::size_type space_pos = old_name.find(' ');
	if (space_pos != std::string::npos) old_name.erase(space_pos);

	if (old_name != name)
	{
		// Remove old name if it has changed
		_packager.remove_sysvar(old_name);

		std::string old_key = tbx::to_lower(old_name);
		std::string new_key = tbx::to_lower(name);

		// Update index map if key changed
		if (old_key != new_key)
		{
			_var_to_index.erase(old_key);
			_var_to_index[new_key] = index;
		}
	}

	// Update variable in package
	_packager.sysvar(name, value);

	// Update display
	std::string display(name);
	display += " ";
	display += value;

	_var_list.item_text(index, display);
}


