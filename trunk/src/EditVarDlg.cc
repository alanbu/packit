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
 * AddVarDlg.cc
 *
 *  Created on: 27-May-2009
 *      Author: alanb
 */

#include "EditVarDlg.h"
#include "SysVarsTab.h"

/**
 * Dialog to add or edit system variables
 *
 * @param sys_vars_tab Tab with the list of existing variables
 *        to which the added/edited variable will update.
 */
EditVarDlg::EditVarDlg(SysVarsTab *sys_vars_tab) :
	_window("EdSysVar"),
	_sys_vars_tab(sys_vars_tab),
	_validate_name(this),
	_validate_value(this)
{
	_name = _window.gadget(2);
	_value = _window.gadget(3);
	_error = _window.gadget(4);
	_ok = _window.gadget(0);

	_name.add_text_changed_listener(&_validate_name);
	_value.add_text_changed_listener(&_validate_value);
	_ok.add_selected_command(this);

	_edit_index = 0;
}

EditVarDlg::~EditVarDlg()
{
}

/**
 * Set the dialog box to edit an existing variable
 */
void EditVarDlg::editing(int index, const std::string &name, const std::string &value)
{
	_edit_index = index;
	_name.text(name);
	_value.text(value);
	_name_error.clear();
	if (value.empty())
	{
		_value_error = "Please enter a value";
		_ok.fade(true);
	} else
		_ok.fade(false);

	_error.text(_value_error);

}

/**
 * Set the dialog box to add a new variable
 */
void EditVarDlg::adding()
{
	_name.text("");
	_value.text("");
	_ok.fade(true);

	_name_error = "Please enter a name and value";
	_value_error = "Please enter a value";
	_error.text(_name_error);
	_edit_index = -1;
}


void EditVarDlg::show(tbx::Window parent)
{
	_window.show_as_menu(parent);
}

/**
 * Update the sys vars tab when OK is pressed
 */
void EditVarDlg::execute()
{
	if (_name_error.empty() && _value_error.empty())
	{
		if (_edit_index == -1)
		{
			_sys_vars_tab->set_var(_name.text(), _value.text());
			_edit_index = _sys_vars_tab->name_index(_name.text());
		} else
		{
			_sys_vars_tab->replace_var(_edit_index, _name.text(), _value.text());
		}
	}
}

/**
 * Update dialog with error due to an invalid name
 */
void EditVarDlg::set_name_error(const char *msg)
{
	if (_name_error.empty() && _value_error.empty()) _ok.fade(true);

	_name_error = msg;
	_error.text(_name_error);
}

/**
 * Clear the name error as it is now valid
 */
void EditVarDlg::clear_name_error()
{
	_name_error.clear();
	_error.text(_value_error);
	if (_value_error.empty()) _ok.fade(false);
}

/**
 * Update the dialog with an error due to an invalid value
 */
void EditVarDlg::set_value_error(const char *msg)
{
	if (_name_error.empty() && _value_error.empty()) _ok.fade(true);

	_value_error = msg;
	_error.text(_value_error);
}

/**
 * Clear the value error as it is now valid
 */
void EditVarDlg::clear_value_error()
{
	_value_error.clear();
	_error.text(_name_error);
	if (_name_error.empty()) _ok.fade(false);
}

/**
 * Check if the given name has already been used
 */
bool EditVarDlg::name_used(const std::string &name) const
{
	int found_index = _sys_vars_tab->name_index(name);
	if (found_index == -1) return false;
	return (found_index != _edit_index);
}

/**
 * Validate that a writeable field with a system variable name is valid
 */
void ValidateSysVarName::text_changed(tbx::TextChangedEvent &event)
{
	std::string name = event.text();
	if (name.empty()) _var_dlg->set_name_error("Name must be entered");
	else if (_var_dlg->name_used(name)) _var_dlg->set_name_error("This name has already been used");
	else _var_dlg->clear_name_error();
}

/**
 * Validate that a writeable field with a system variable value is valid
 */
void ValidateSysVarValue::text_changed(tbx::TextChangedEvent &event)
{
	std::string value = event.text();
	if (value.empty()) _var_dlg->set_value_error("Name must be entered");
	else
	{
		std::string::size_type lpos = value.find('<');
		std::string::size_type rpos = value.find('>');
		const char *errmsg = 0;

		if (lpos == std::string::npos)
		{
			if (rpos != std::string::npos) errmsg = "Missing '<' from value";
		} else
		{
			while (lpos != std::string::npos && !errmsg)
			{
				if (rpos == std::string::npos)
				{
					errmsg = "Missing '>' from value";
				} else	if (rpos < lpos)
				{
					errmsg = "'>' before matching '<' in value";
				}
				else
				{
					lpos = value.find('<', rpos+1);
					rpos = value.find('>', rpos+1);
				}
			}
			if (!errmsg && rpos != std::string::npos)
			{
				errmsg = "Missing '<' from value";
			}
		}

		if (errmsg) _var_dlg->set_value_error(errmsg);
		else _var_dlg->clear_value_error();
	}
}
