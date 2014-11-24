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

#ifndef SPRITESTAB_H
#define SPRITESTAB_H

#include "tbx/window.h"
#include "tbx/command.h"
#include "tbx/scrolllist.h"
#include "ITab.h"
#include "Packager.h"

class EditSpriteDlg;

/**
 * Class to display and allow editing of sprite information
 */
class SpritesTab : public ITab
{
    Packager &_packager;
	tbx::Window _window;
	tbx::ScrollList _sprite_list;
	tbx::CommandMethod<SpritesTab> _add_sprite_command;
	tbx::CommandMethod<SpritesTab> _edit_sprite_command;
	tbx::CommandMethod<SpritesTab> _remove_sprite_command;
	tbx::CommandMethod<SpritesTab> _clear_sprites_command;
	tbx::CommandMethod<SpritesTab> _scan_sprites_command;
	std::map<std::string, int> _sprite_to_index;
	EditSpriteDlg *_edit_sprite_dlg;

    private:
    	void add_sprite();
    	void edit_sprite();
    	void remove_sprite();
    	void clear_sprites();
    	void scan_sprites();
    	void scan_sprites(std::istream &is);

    	void set_sprite(const char *sprite_name, const char *location);

    public:
        SpritesTab(tbx::Window window, Packager &packager);
        virtual ~SpritesTab();

        virtual void package_loaded();

        int name_index(const std::string &name) const;

    	void set_sprite(const std::string &name, const std::string &location)
    	{
    		set_sprite(name.c_str(), location.c_str());
    	}

    	void replace_sprite(int index, const std::string &name, const std::string &location);

        /**
         * Return leaf name of file with help for this tab
         */
        virtual std::string help_name() const {return "sprites";}

};

#endif
