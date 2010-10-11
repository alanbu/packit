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
 * EditSpriteDlg.h
 *
 *  Created on: 27-May-2009
 *      Author: alanb
 */

#ifndef EDITSPRITEDLG_H_
#define EDITSPRITEDLG_H_

#include "tbx/window.h"
#include "tbx/writablefield.h"
#include "tbx/actionbutton.h"
#include "tbx/textchangedlistener.h"
#include "tbx/displayfield.h"
#include "tbx/command.h"

class SpritesTab;
class EditSpriteDlg;

/**
 * Class to validate Sprite name is valid
 */
class ValidateSpriteName : public tbx::TextChangedListener
{
	EditSpriteDlg *_sprites_dlg;
public:
	ValidateSpriteName(EditSpriteDlg *sprites_dlg) : _sprites_dlg(sprites_dlg) {}

	virtual void text_changed(tbx::TextChangedEvent &event);
};

/**
 * Class to validate Sprite location is valid
 */
class ValidateSpriteLocation : public tbx::TextChangedListener
{
	EditSpriteDlg *_sprites_dlg;
public:
	ValidateSpriteLocation(EditSpriteDlg *sprites_dlg) : _sprites_dlg(sprites_dlg) {}

	virtual void text_changed(tbx::TextChangedEvent &event);
};


/**
 * Class to handle adding and editing of variables variables dialog
 */
class EditSpriteDlg : public tbx::Command
{
	tbx::Window _window;
	SpritesTab *_sprites_tab;
	tbx::WritableField _name;
	tbx::WritableField _location;
	tbx::DisplayField _error;
	tbx::ActionButton _ok;
	ValidateSpriteName _validate_name;
	ValidateSpriteLocation _validate_location;
	std::string _name_error;
	std::string _location_error;
	int _edit_index;

public:
	EditSpriteDlg(SpritesTab *sprites_tab);
	virtual ~EditSpriteDlg();

	void editing(int index, const std::string &name, const std::string &location);
	void adding();

	void show(tbx::Window parent);

	void set_name_error(const char *msg);
	void clear_name_error();
	void set_location_error(const char *msg);
	void clear_location_error();

	bool name_used(const std::string &name) const;

	virtual void execute();
};



#endif /* EDITSPRITEDLG_H_ */
