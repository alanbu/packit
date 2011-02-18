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
 * DependsTab.cc
 *
 *  Created on: 22-May-2009
 *      Author: alanb
 */

#include "DependsTab.h"
#include "MainWindow.h"
#include "tbx/writablefield.h"

DependsTab::DependsTab(MainWindow *main, tbx::Window window, Packager &packager) :
   _packager(packager)
{
   // Get fields
   tbx::WritableField depends = window.gadget(2);
   tbx::WritableField recommends = window.gadget(4);
   tbx::WritableField suggests = window.gadget(6);
   tbx::WritableField conflicts = window.gadget(8);

   // Set bindings
   main->set_binding(DEPENDS, depends);
   main->set_binding(RECOMMENDS, recommends);
   main->set_binding(SUGGESTS, suggests);
   main->set_binding(CONFLICTS, conflicts);
}

void DependsTab::package_loaded()
{

}
