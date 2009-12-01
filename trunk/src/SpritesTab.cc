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
 * SpritesTab.cc
 *
 *  Created on: 28-May-2009
 *      Author: alanb
 */

#include "SpritesTab.h"
#include "eventids.h"
#include "EditSpriteDlg.h"

#include <fstream>
#include <string.h>

#include "tbx/path.h"
#include "tbx/stringutils.h"

SpritesTab::SpritesTab(tbx::Window window, Packager &packager) :
	_packager(packager),
	_window(window),
	_add_sprite_command(this, &SpritesTab::add_sprite),
    _edit_sprite_command(this, &SpritesTab::edit_sprite),
    _remove_sprite_command(this, &SpritesTab::remove_sprite),
    _clear_sprites_command(this, &SpritesTab::clear_sprites),
    _scan_sprites_command(this, &SpritesTab::scan_sprites)
{
	_sprite_list = window.gadget(0);

	window.add_command(EID_SPRITES_ADD, &_add_sprite_command);
	window.add_command(EID_SPRITES_EDIT, &_edit_sprite_command);
	window.add_command(EID_SPRITES_REMOVE, &_remove_sprite_command);
	window.add_command(EID_SPRITES_CLEAR, &_clear_sprites_command);
	window.add_command(EID_SPRITES_SCAN_SPRITES, &_scan_sprites_command);

	_edit_sprite_dlg = 0;
}

SpritesTab::~SpritesTab()
{
	delete _edit_sprite_dlg;
}

void SpritesTab::package_loaded()
{
	Packager::SpritesMap::const_iterator i;

	for (i = _packager.sprites().begin(); i != _packager.sprites().end(); ++i)
	{
		set_sprite((*i).first.c_str(), (*i).second.c_str());
	}
}

/**
 * Return index for a name
 */
int SpritesTab::name_index(const std::string &name) const
{
	std::string map_name(tbx::to_lower(name));
	std::map<std::string, int>::const_iterator found = _sprite_to_index.find(map_name);
	return (found != _sprite_to_index.end()) ? found->second : -1;
}

/**
 * Add a new sprite
 */
void SpritesTab::add_sprite()
{
	if (_edit_sprite_dlg == 0) _edit_sprite_dlg = new EditSpriteDlg(this);
	_edit_sprite_dlg->adding();
	printf("pre show\n");
	_edit_sprite_dlg->show(_window);
	printf("post show\n");
}

/**
 * Edit a sprite
 */
void SpritesTab::edit_sprite()
{
	int index = _sprite_list.first_selected();
	if (index >= 0)
	{
		std::string name = _sprite_list.item_text(index);
		std::string location;
		std::string::size_type space_pos = name.find(' ');
		if (space_pos != std::string::npos)
		{
			location = name.substr(space_pos+1);
			name.erase(space_pos);
		}

		if (_edit_sprite_dlg == 0) _edit_sprite_dlg = new EditSpriteDlg(this);
		_edit_sprite_dlg->editing(index, name, location);
		_edit_sprite_dlg->show(_window);
	}
}

/**
 * Remove a sprite
 */
void SpritesTab::remove_sprite()
{
	int index = _sprite_list.first_selected();
	if (index >= 0)
	{
		_sprite_list.delete_item(index);

		std::map<std::string, int>::iterator i, found = _sprite_to_index.end();
		for (i = _sprite_to_index.begin(); i != _sprite_to_index.end(); ++i)
		{
			if ((*i).second == index) found = i;
			else if ((*i).second > index)
			{
				(*i).second -= 1;
			}
		}
		if (found != _sprite_to_index.end()) _sprite_to_index.erase(found);
	}
}

/**
 * Delete a sprite
 */
void SpritesTab::clear_sprites()
{
	_sprite_list.clear();
	_sprite_to_index.clear();
	_packager.clear_sprites();
}

/**
 * Scan the !Sprites file for a sprite
 */
void SpritesTab::scan_sprites()
{
	std::string zipfile, zipitem;
	if (_packager.files_in_zip(zipfile, zipitem))
	{
		zipitem += "/!Sprites";
		std::string buf;
		if (_packager.read_zip_item(zipfile, zipitem, buf))
		{
			std::istringstream is(buf);
			scan_sprites(is);
		}
	} else
	{
		tbx::Path path(_packager.item_to_package(), "!Sprites");
		if (path.exists())
		{
			std::ifstream is(path);
			if (is) scan_sprites(is);
		}
	}
}

/**
 * Scan stream opened on a !Sprites file
 */
void SpritesTab::scan_sprites(std::istream &is)
{
	int num_sprites;
	if (!is.read((char *)&num_sprites, 4)) return;
	if (num_sprites == 0) return;

	std::string location("<Obey$Dir>.!Sprites");

	int offset;
	char name[13];

	is.read((char *)&offset, 4); // Read offset to first sprite
	is.seekg(offset-12, std::ios_base::cur); // Move to first sprite

	while (num_sprites-- && is)
	{
		is.read((char *)&offset, 4); // Read offset to next sprite
		is.read(name, 12); // Read name
		name[12] = 0;
		printf("Found sprite %s\n", name);
		is.seekg(offset-16, std::ios_base::cur); // Move to next sprite
		set_sprite(name, location);
	}

}

/**
 * Set new value or add a sprite to the scroll list
 */
void SpritesTab::set_sprite(const char *sprite_name, const char *location)
{
	_packager.sprite(sprite_name, location);

	std::string name;
	const char *p = sprite_name;
	while (*p) name += std::tolower(*p++);

	std::string display(sprite_name);
	display += " ";
	display += location;

	std::map<std::string, int>::iterator found = _sprite_to_index.find(name);
	if (found != _sprite_to_index.end())
	{
		_sprite_list.item_text((*found).second, display);
	} else
	{
		int index= _sprite_to_index.size();
		_sprite_list.add_item(display);
		_sprite_to_index[name] = index;
	}
}

/**
 * Replace a variable that has been modified
 */
void SpritesTab::replace_sprite(int index, const std::string &name, const std::string &location)
{
	std::string old_name = _sprite_list.item_text(index);
	std::string::size_type space_pos = old_name.find(' ');
	if (space_pos != std::string::npos) old_name.erase(space_pos);

	if (old_name != name)
	{
		// Remove old name if it has changed
		_packager.remove_sprite(old_name);

		std::string old_key = tbx::to_lower(old_name);
		std::string new_key = tbx::to_lower(name);

		// Update index map if key changed
		if (old_key != new_key)
		{
			_sprite_to_index.erase(old_key);
			_sprite_to_index[new_key] = index;
		}
	}

	// Update variable in package
	_packager.sprite(name, location);

	// Update display
	std::string display(name);
	display += " ";
	display += location;

	_sprite_list.item_text(index, display);
}


