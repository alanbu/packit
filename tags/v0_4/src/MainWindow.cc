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

#include "MainWindow.h"
#include "Tabs.h"

#include "MainTab.h"
#include "DescTab.h"
#include "FilesTab.h"
#include "CopyrightTab.h"
#include "DependsTab.h"
#include "SysVarsTab.h"
#include "SpritesTab.h"
#include "eventids.h"

#include "SavePackage.h"
#include "ModifiedPrompt.h"

#include "tbx/path.h"
#include "tbx/reporterror.h"
#include "tbx/deleteonhidden.h"
#include "tbx/hourglass.h"

#include <sstream>

using namespace tbx;


MainWindow::MainWindow() : _window("PackIt"),
   _next_error_command(this, &MainWindow::next_error),
   _save_command(this, &MainWindow::save),
   _tab_help_command(this, &MainWindow::show_tab_help)
{
   _current_error = 0;

   Tabs tabs = _window.gadget(1);
   _error_count = _window.gadget(3);
   _error_message = _window.gadget(5);

   Window main_wnd = tabs.window_from_tab(0);
   Window desc_wnd = tabs.window_from_tab(1);
   Window files_wnd = tabs.window_from_tab(2);
   Window copyright_wnd = tabs.window_from_tab(3);
   Window depends_wnd = tabs.window_from_tab(4);
   Window sysvars_wnd = tabs.window_from_tab(5);
   Window sprites_wnd = tabs.window_from_tab(6);

   // Clear bindings - tabs will handle the transfer of
   // data themselves.
   for (int j = 0; j < NUM_ITEMS; j++)
      _binding[j] = 0;

   _tabs[0] = new MainTab(this, main_wnd, _packager);
   _tabs[1] = new DescTab(this, desc_wnd, _packager);
   _tabs[2] = new FilesTab(this, files_wnd, _packager);
   _tabs[3] = new CopyrightTab(this, copyright_wnd, _packager);
   _tabs[4] = new DependsTab(this, depends_wnd, _packager);
   _tabs[5] = new SysVarsTab(sysvars_wnd, _packager);
   _tabs[6] = new SpritesTab(sprites_wnd, _packager);

   _packager.set_modified_listener(this);
   _packager.set_error_listener(this);

   _window.add_close_window_listener(this);
   _window.add_command(EID_NEXT_ERROR, &_next_error_command);
   _window.add_command(EID_SAVE, &_save_command);
   _window.add_command(EID_TAB_HELP, &_tab_help_command);

   _window.client_handle(this);
}

/**
 * Delete main window
 */
MainWindow::~MainWindow()
{
	// Delete will dump any reference objects as well.
	_window.delete_object();

	for (int tab = 0; tab < num_tabs; tab++)
		delete _tabs[tab];
}

/**
 * Static function to return the main window from its toolbox window
 */
MainWindow *MainWindow::from_window(tbx::Window window)
{
	return reinterpret_cast<MainWindow *>(window.client_handle());
}

/**
 * Load the file
 *
 * Detects if it's a package or a file to be packaged.
 *
 * Note: Also fails if the file is a package containing fields
 * packit does not understand.
 *
 * @param file_name name of file to load
 * @param file_type file type
 * @returns true if file could be loaded/used.
 */
bool MainWindow::load_file(std::string file_name, int file_type)
{
	tbx::Hourglass hg;
	try
	{
		if (_packager.load(file_name))
		{
			_title = file_name;
			_window.title(file_name);
		} else
		{
			tbx::Path fn(file_name);
			std::string name(fn.leaf_name());
			if (name[0] == '!') name.erase(0,1);

			_title = name + "_-1";
			_packager.package_name(name);
			_packager.item_to_package(file_name);
		}
	} catch(PackageFormatException e)
	{
		tbx::Window load_failed("LoadFailed");
		tbx::DisplayField fn = load_failed.gadget(1);
		tbx::DisplayField em = load_failed.gadget(3);
		fn.text(file_name);
		em.text(e.message());
		load_failed.add_has_been_hidden_listener(new DeleteObjectOnHidden());
		load_failed.show();
		return false;
	}

    // Load all bound fields
    for (int j = 0; j < NUM_ITEMS; j++)
    {
    	if (_binding[j] != 0) _binding[j]->update_source();
    }

    for (int tab = 0 ; tab < num_tabs; tab++)
    {
       _tabs[tab]->package_loaded();
    }


    if (_packager.error_count())
    {
       std::ostringstream ss;
       ss << _packager.error_count();
       _error_count.text(ss.str());
       // Show first error
       _current_error = _packager.first_error();

       _error_message.text(_packager.item_name(_current_error)
               + " " + _packager.error_text(_current_error));
    }

    _packager.modified(false);

    return true;
}

/**
 * Show the main window a set focus to first tab
 */
void MainWindow::show()
{
	_window.show();
    _tabs[0]->switched_to();
}


/**
 * Save packages
 */
bool MainWindow::save_file(std::string filename)
{
	// Ensure any fields that are not updated in real time
	// are updated
	for (int j = 0; j < num_tabs; j++)
	{
		_tabs[j]->pre_save();
	}

	return _packager.save(filename);
}

/**
 * Set binding between writable field and an item
 */
void MainWindow::set_binding(PackageItem item, WritableField writeable)
{
    _binding[item] = new TextBinding(new WritableTextEndPoint(writeable),
         _packager.get_binding(item)
    );
}

/**
 * Set binding between a numberrange and an item
 */
void MainWindow::set_binding(PackageItem item, NumberRange nr)
{
    _binding[item] = new TextBinding(new NumberRangeEndPoint(nr),
         _packager.get_binding(item)
    );
}

/**
 * Set binding between string set and an item
 */
void MainWindow::set_binding(PackageItem item, tbx::StringSet ss)
{
    _binding[item] = new TextBinding(new StringSetEndPoint(ss),
         _packager.get_binding(item)
    );
}

/**
 * Update binding for given item.
 *
 * This is sometime necessary when setting a field does not generate
 * the changed event.
 */
void MainWindow::update_binding(PackageItem item)
{
	_binding[item]->update_target();
}

/**
 * Called when package modified flag has changed
 */
void MainWindow::modified_changed(bool new_value)
{
	if (new_value)
	{
		_window.title(_title + " *");
	} else
		_window.title(_title);
}

void MainWindow::title(std::string title)
{
	if (title != _title)
	{
		_title = title;

		modified_changed(_packager.modified()); // Force title update
	}
}


/**
 * Called by package when item goes to an error state
 */
void MainWindow::on_item_error(PackageItem item)
{
    std::ostringstream ss;
    ss << _packager.error_count();
    _error_count.text(ss.str());

    _current_error = item;
    _error_message.text(_packager.item_name(_current_error)
             + " " + _packager.error_text(_current_error));
}

/**
 * Called by packager when item becomes valid
 */
void MainWindow::on_item_valid(PackageItem item)
{
    std::ostringstream ss;
    ss << _packager.error_count();
    _error_count.text(ss.str());
   _current_error = item;

   _error_message.text(_packager.item_name(_current_error)
              + " is now valid");
}

/**
 * Show the next error in the status display
 */
void MainWindow::next_error()
{
	_current_error = _packager.next_error(_current_error);
	if (_current_error >= 0)
	{
	    _error_message.text(_packager.item_name(_current_error)
	             + " " + _packager.error_text(_current_error));
	} else
	{
	    _error_message.text("All items are valid");
	}
}

/**
 * Save document, showing saveas dialog if no full title.
 */
void MainWindow::save()
{
	if (_title.find_first_of(":.") == std::string::npos)
	{
		// Not a full path so show save as dialog
		SavePackage::show(_window);
	} else if (save_file(_title))
	{
		_packager.modified(false);
	}
}

/**
 * Show save as for this window
 */
void MainWindow::show_save()
{
	SavePackage::show(_window);
}

/**
 * Close window request from window
 */
void MainWindow::close_window(const tbx::EventInfo &close_event)
{
	if (_packager.modified()) ModifiedPrompt::show(this);
	else delete this;
}

/**
 * Tab selected so inform tab class.
 *
 * Usually tab class would set the focus
 */
void MainWindow::tab_selected(int old_index, int new_index)
{
	if (new_index != -1) _tabs[new_index]->switched_to();
}

/**
 * Show Help for current tab
 */
void MainWindow::show_tab_help()
{
	std::string cmd("Filer_Run <PackIt$Dir>.html.");
    Tabs tabs = _window.gadget(1);
    int index = tabs.selected();
    if (index != -1)
    {
    	cmd += _tabs[index]->help_name();
    	cmd += "/htm";
    	_kernel_oscli(cmd.c_str());
    }
}
