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
#include "tbx/WriteableField.h"
#include "tbx/StringSet.h"


FilesTab::FilesTab(MainWindow *main,
 tbx::Window window, Packager &packager) : _packager(packager)
{
   // Fields from dialog
   tbx::WriteableField item_name = window.gadget(1);
   tbx::StringSet install_to = window.gadget(3);

   // Set bindings
   main->set_binding(ITEM_TO_PACKAGE, item_name);
   main->set_binding(INSTALL_TO, install_to);
}

void FilesTab::package_loaded()
{
}
