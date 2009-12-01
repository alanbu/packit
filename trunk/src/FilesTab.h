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

#ifndef FILESTAB_H
#define FILESTAB_H

#include "tbx/Window.h"
#include "tbx/WriteableField.h"
#include "ITab.h"
#include "Packager.h"

class MainWindow;

class FilesTab : public ITab
{
    Packager &_packager;
    tbx::WriteableField _item_name;

    public:
       FilesTab(MainWindow *main, tbx::Window window, Packager &packager);
       virtual ~FilesTab() {}

       virtual void package_loaded();

       /**
        * Return leaf name of file with help for this tab
        */
       virtual std::string help_name() const {return "files";}

};


#endif
