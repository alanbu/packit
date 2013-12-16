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

/**
 * Class to handle tab to specify
 */

#ifndef SYSVARSTAB_H
#define SYSVARSTAB_H

#include "tbx/Window.h"
#include "tbx/command.h"
#include "tbx/scrolllist.h"
#include "ITab.h"
#include "Packager.h"

#include <map>
#include <istream>

class EditVarDlg;

class SysVarsTab : public ITab
{
    Packager &_packager;
    tbx::Window _window;
    tbx::ScrollList _var_list;
    tbx::CommandMethod<SysVarsTab> _add_var_command;
    tbx::CommandMethod<SysVarsTab> _edit_var_command;
    tbx::CommandMethod<SysVarsTab> _remove_var_command;
    tbx::CommandMethod<SysVarsTab> _clear_vars_command;
    tbx::CommandMethod<SysVarsTab> _scan_boot_command;
    std::map<std::string, int> _var_to_index;
    EditVarDlg *_edit_var_dlg;

private:
	void add_var();
	void edit_var();
	void remove_var();
	void clear_vars();
	void scan_boot();
	void scan_boot(std::istream &is);
	char *check_if(char *p);

	void set_var(const char *var_name, const char *value, bool update_package = true);


public:
    SysVarsTab(tbx::Window window, Packager &packager);
    virtual ~SysVarsTab();

    virtual void package_loaded();

    int name_index(const std::string &name) const;

	void set_var(const std::string &name, const std::string &value)
	{
		set_var(name.c_str(), value.c_str());
	}

	void replace_var(int index, const std::string &name, const std::string &value);

    /**
     * Return leaf name of file with help for this tab
     */
    virtual std::string help_name() const {return "sysvars";}
};


#endif
