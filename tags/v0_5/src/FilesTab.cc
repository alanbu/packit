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
 * Class to handle tab to specifiy what files are packaged and
 * where to
 */

#include "MainWindow.h"
#include "FilesTab.h"
#include "Packager.h"
#include "tbx/stringSet.h"


FilesTab::FilesTab(MainWindow *main,
 tbx::Window window, Packager &packager) : _packager(packager)
{
   // Fields from dialog
   tbx::WritableField item_name = window.gadget(1);
   tbx::StringSet install_to = window.gadget(3);
   tbx::StringSet component_flags = window.gadget(5);

   _file_drop = new SetNameToDroppedFile(_packager, item_name);
   item_name.add_loader(_file_drop);

   // Set bindings
   main->set_binding(ITEM_TO_PACKAGE, item_name);
   main->set_binding(INSTALL_TO, install_to);
   main->set_binding(COMPONENT_FLAGS, component_flags);
}

void FilesTab::package_loaded()
{
}

/**
 * Set name to name of file dropped from filer
 */
bool FilesTab::SetNameToDroppedFile::load_file(tbx::LoadEvent &event)
{
	if (event.from_filer())
	{
		_item_name.text(event.file_name());
		_packager.item_to_package(event.file_name());
	}

	return false;
}
