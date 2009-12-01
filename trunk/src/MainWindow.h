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


#include "tbx/window.h"
#include "tbx/displayfield.h"
#include "tbx/writeablefield.h"
#include "tbx/numberrange.h"
#include "tbx/StringSet.h"
#include "tbx/command.h"
#include "tbx/closewindowlistener.h"

#include "ITab.h"
#include "Packager.h"
#include "Binding.h"
#include "Tabs.h"

const int num_tabs = 7;

class MainWindow :
	public PackageErrorListener,
	public PackageModifiedListener,
	public tbx::CloseWindowListener,
	public TabSelectionListener
{
    private:
       tbx::Window _window;
       Packager _packager;
       std::string _title;

       ITab *_tabs[num_tabs];
       tbx::DisplayField _error_count;
       tbx::DisplayField _error_message;
       int _current_error;

       TextBinding *_binding[NUM_ITEMS];

       tbx::CommandMethod<MainWindow> _next_error_command;
       tbx::CommandMethod<MainWindow> _save_command;
       tbx::CommandMethod<MainWindow> _tab_help_command;

    public:
       MainWindow();
       ~MainWindow();

       static MainWindow *from_window(tbx::Window window);
       Packager &packager() {return _packager;}

       const std::string &title() const {return _title;}
       void title(std::string title);

       void show() {_window.show();}

       void set_binding(PackageItem item, tbx::WriteableField writeable);
       void set_binding(PackageItem item, tbx::NumberRange nr);
       void set_binding(PackageItem item, tbx::StringSet ss);

       bool load_file(std::string file_name, int file_type);
       bool save_file(std::string file_name);

       void save();
       void show_save();
       void next_error();

       void show_tab_help();

       virtual void modified_changed(bool new_value);
       virtual void on_item_error(PackageItem item);
       virtual void on_item_valid(PackageItem item);

       virtual void close_window(tbx::Window window);

       virtual void tab_selected(int old_index, int new_index);

};
