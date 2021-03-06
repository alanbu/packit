/*********************************************************************
* Copyright 2019 Alan Buckley
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
* OSDependsMenu.cpp
 *
 *  Created on: 24 Mar 2019
 *      Author: alanb
 */

#include "OSDependsMenu.h"
#include "eventids.h"
#include "MainWindow.h"
#include "DependsTab.h"
#include "tbx/application.h"
#include "tbx/window.h"
#include "tbx/writablefield.h"
#include "tbx/res/resmenu.h"
#include <vector>
#include <algorithm>
#include "swis.h"


OSDependsMenu::OSDependsMenu() : _item_count(0)
{
}

OSDependsMenu::~OSDependsMenu()
{
}

/**
 * Menu has been autocreated
 */
void OSDependsMenu::auto_created(std::string template_name, tbx::Object object)
{
	_menu = object;
	_menu.add_about_to_be_shown_listener(this);
	_menu.add_user_event_listener(EID_ADD_DEPENDENCY, this);
}

/**
 * Set up list of packages
 */
void OSDependsMenu::about_to_be_shown(tbx::AboutToBeShownEvent &event)
{
	// For speed just set up once
	if (_item_count > 0) return;

    try
    {
	   std::string name;
	   std::vector<std::string> names;
	   std::string keyword;
	   _kernel_oserror *err = 0;
	   _kernel_swi_regs regs;
	   regs.r[0] = 19; // Enumerate ROM modules
	   regs.r[1] = 0;  // start full enumeration
	   regs.r[2] = -1; // start full enumeration

	   while (err == 0)
	   {
		   err = _kernel_swi(OS_Module, &regs, &regs);
		   if (err == 0 && regs.r[4] != -1)
		   {
			   name = reinterpret_cast<char *>(regs.r[3]);
			   names.push_back(name);
		   }
	   }

 	   _item_count = names.size();
 	   if (_item_count == 0)
 	   {
 		   // Didn't find any names so just fade default entry
 		   _item_count = 1;
		   _menu.item(0).fade(true);
 	   } else
 	   {
 		   // Get a sorted list
 		   std::sort(names.begin(), names.end());
		   _menu.item(0).text(names[0]);

		   tbx::res::ResMenu resMenu = tbx::app()->resource("DependsMenu");
		   tbx::res::ResMenuItem newItem = resMenu.item(0);

		   for (unsigned int i = 10; i < _item_count; ++i)
		   {
			   newItem.text(names[i]);
			   newItem.component_id(i);
			   _menu.add(newItem);
		   }
 	   }

   } catch (...)
   {
   	// Ignore any exceptions. We just can't add the values.
   }
}

/**
 * Process user event to add the dependency to the relevant writeable
 * field.
 */
void OSDependsMenu::user_event(tbx::UserEvent &event)
{
	tbx::MenuItem menu_item(event.id_block().self_component());
	// We are parent of this menu so we can get our menu item from there
	tbx::Window window(event.id_block().parent_object());
	// Parent gadgets id is arrange so subtracting 16 gives corresponding
	// write able field id.
	tbx::ComponentId gadget_id = event.id_block().parent_component().id() - 16;
	tbx::WritableField fld_to_add_to(window.gadget(gadget_id));

	std::string text = fld_to_add_to.text();
	if (text.empty())
	{
		fld_to_add_to.text(menu_item.text());
	} else
	{
		std::string pack_text(menu_item.text());
		std::string pack_name(pack_text);
		std::string::size_type pos = pack_text.find(' ');
		if (pos != std::string::npos) pack_name.erase(pos);
		std::string::size_type name_size = pack_name.size();

		bool found = false;
		pos = 0;
		while (!found && pos != std::string::npos)
		{
			pos = text.find(pack_name);
			if (pos != std::string::npos)
			{
				if (pos + name_size == text.size()
					|| text[pos+name_size] == ' '
					|| text[pos+name_size] == '('
					|| text[pos+name_size] == ',')
				{
					found = true;
				}
			}
		}
		if (found)
		{
			std::string::size_type replace_size = name_size;
			while (pos + replace_size < text.size()
				   && text[pos + replace_size] != ',')
			{
				replace_size++;
			}
			text.replace(pos, replace_size, pack_text);
		} else
		{
			text += ", ";
			text += pack_text;
		}
		fld_to_add_to.text(text);
	}

	// Setting the text does not always generate the writable field changed
	// event so the following ensures the binding is run.
	MainWindow *main = MainWindow::from_window(event.id_block().ancestor_object());
	if (main) main->update_binding(DependsTab::item_from_gadget(gadget_id));
}

