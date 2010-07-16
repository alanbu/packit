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

#include "EditSpriteDlg.h"
#include "SpritesTab.h"

/**
 * Dialog to add or edit sprite locations
 *
 * @param sprites_tab Tab with the list of existing sprites
 *        to which the added/edited sprite will update.
 */
EditSpriteDlg::EditSpriteDlg(SpritesTab *sprites_tab) :
	_window("EdSprite"),
	_sprites_tab(sprites_tab),
	_validate_name(this),
	_validate_location(this)
{
	_name = _window.gadget(2);
	_location = _window.gadget(3);
	_error = _window.gadget(4);
	_ok = _window.gadget(0);

	_name.add_text_changed_listener(&_validate_name);
	_location.add_text_changed_listener(&_validate_location);
	_ok.add_selected_command(this);

	_edit_index = 0;
}

EditSpriteDlg::~EditSpriteDlg()
{
}

/**
 * Set the dialog box to edit an existing sprite
 */
void EditSpriteDlg::editing(int index, const std::string &name, const std::string &location)
{
	_edit_index = index;
	_name.text(name);
	_location.text(location);
	_name_error.clear();
	if (location.empty())
	{
		_location_error = "Please enter a value";
		_ok.fade(true);
	} else
		_ok.fade(false);

	_error.text(_location_error);

}

/**
 * Set the dialog box to add a new variable
 */
void EditSpriteDlg::adding()
{
	_name.text("");
	_location.text("");
	_ok.fade(true);

	_name_error = "Please enter a name and value";
	_location_error = "Please enter a value";
	_error.text(_name_error);
	_edit_index = -1;
}


void EditSpriteDlg::show(tbx::Window parent)
{
	_window.show_as_menu(parent);
}

/**
 * Update the sys vars tab when OK is pressed
 */
void EditSpriteDlg::execute()
{
	if (_name_error.empty() && _location_error.empty())
	{
		if (_edit_index == -1)
		{
			_sprites_tab->set_sprite(_name.text(), _location.text());
			_edit_index = _sprites_tab->name_index(_name.text());
		} else
		{
			_sprites_tab->replace_sprite(_edit_index, _name.text(), _location.text());
		}
	}
}

/**
 * Update dialog with error due to an invalid name
 */
void EditSpriteDlg::set_name_error(const char *msg)
{
	if (_name_error.empty() && _location_error.empty()) _ok.fade(true);

	_name_error = msg;
	_error.text(_name_error);
}

/**
 * Clear the name error as it is now valid
 */
void EditSpriteDlg::clear_name_error()
{
	_name_error.clear();
	_error.text(_location_error);
	if (_location_error.empty()) _ok.fade(false);
}

/**
 * Update the dialog with an error due to an invalid location
 */
void EditSpriteDlg::set_location_error(const char *msg)
{
	if (_name_error.empty() && _location_error.empty()) _ok.fade(true);

	_location_error = msg;
	_error.text(_location_error);
}

/**
 * Clear the location error as it is now valid
 */
void EditSpriteDlg::clear_location_error()
{
	_location_error.clear();
	_error.text(_name_error);
	if (_name_error.empty()) _ok.fade(false);
}

/**
 * Check if the given name has already been used
 */
bool EditSpriteDlg::name_used(const std::string &name) const
{
	int found_index = _sprites_tab->name_index(name);
	if (found_index == -1) return false;
	return (found_index != _edit_index);
}

/**
 * Validate that a writeable field with a sprite name is valid
 */
void ValidateSpriteName::text_changed(tbx::TextChangedEvent &event)
{
	std::string name = event.text();
	if (name.empty()) _sprites_dlg->set_name_error("Name must be entered");
	else if (_sprites_dlg->name_used(name)) _sprites_dlg->set_name_error("This name has already been used");
	else _sprites_dlg->clear_name_error();
}

/**
 * Validate that a writeable field with a sprite location is valid
 */
void ValidateSpriteLocation::text_changed(tbx::TextChangedEvent &event)
{
	std::string value = event.text();
	if (value.empty()) _sprites_dlg->set_location_error("Name must be entered");
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

		if (errmsg) _sprites_dlg->set_location_error(errmsg);
		else _sprites_dlg->clear_location_error();
	}
}
