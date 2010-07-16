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
 * EditVarDlg.h
 *
 *  Created on: 27-May-2009
 *      Author: alanb
 */

#ifndef EDITVARDLG_H_
#define EDITVARDLG_H_

#include "tbx/window.h"
#include "tbx/writeablefield.h"
#include "tbx/actionbutton.h"
#include "tbx/textchangedlistener.h"
#include "tbx/displayfield.h"
#include "tbx/command.h"

class SysVarsTab;
class EditVarDlg;

/**
 * Class to validate System variable name is valid
 */
class ValidateSysVarName : public tbx::TextChangedListener
{
	EditVarDlg *_var_dlg;
public:
	ValidateSysVarName(EditVarDlg *var_dlg) : _var_dlg(var_dlg) {}

	virtual void text_changed(tbx::TextChangedEvent &event);
};

/**
 * Class to validate System variable value is valid
 */
class ValidateSysVarValue : public tbx::TextChangedListener
{
	EditVarDlg *_var_dlg;
public:
	ValidateSysVarValue(EditVarDlg *var_dlg) : _var_dlg(var_dlg) {}

	virtual void text_changed(tbx::TextChangedEvent &event);
};


/**
 * Class to handle adding and editing of variables variables dialog
 */
class EditVarDlg : public tbx::Command
{
	tbx::Window _window;
	SysVarsTab *_sys_vars_tab;
	tbx::WriteableField _name;
	tbx::WriteableField _value;
	tbx::DisplayField _error;
	tbx::ActionButton _ok;
	ValidateSysVarName _validate_name;
	ValidateSysVarValue _validate_value;
	std::string _name_error;
	std::string _value_error;
	int _edit_index;

public:
	EditVarDlg(SysVarsTab *sys_vars_tab);
	virtual ~EditVarDlg();

	void editing(int index, const std::string &name, const std::string &value);
	void adding();

	void show(tbx::Window parent);

	void set_name_error(const char *msg);
	void clear_name_error();
	void set_value_error(const char *msg);
	void clear_value_error();

	bool name_used(const std::string &name) const;

	virtual void execute();
};



#endif /* ADDVARDLG_H_ */
