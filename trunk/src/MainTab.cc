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
 * Class to handle tab to specify main details for package.
 *
 * i.e. name, version etc
 */


#include "MainTab.h"
#include "MainWindow.h"

MainTab::MainTab(MainWindow *main, tbx::Window window, Packager &packager) :
	_packager(packager),
	_name_changed(main),
	_version_changed(main),
	_rpversion_changed(main)
{
   // Get fields
   tbx::WritableField package = window.gadget(3);
   tbx::WritableField version = window.gadget(5);
   tbx::NumberRange package_version = window.gadget(7);

   tbx::StringSet section = window.gadget(9);
   tbx::StringSet priority = window.gadget(11);
   tbx::WritableField maintainer = window.gadget(15);
   tbx::WritableField standards_version = window.gadget(17);

   // Set bindings
   main->set_binding(PACKAGE_NAME, package);
   main->set_binding(VERSION, version);
   main->set_binding(PACKAGE_VERSION, package_version);
   main->set_binding(SECTION, section);
   main->set_binding(PRIORITY, priority);
   main->set_binding(MAINTAINER, maintainer);
   main->set_binding(STANDARDS_VERSION, standards_version);

   // Keep window title in line with name and version fields
   package.add_text_changed_listener(&_name_changed);
   version.add_text_changed_listener(&_version_changed);
   package_version.add_value_changed_listener(&_rpversion_changed);

   _initial_focus = package;
}

/**
 * Initial load of package done so set fields
 */
void MainTab::package_loaded()
{
}

/**
 * Set focus when this tab is switched to
 */
void MainTab::switched_to()
{
	_initial_focus.focus();
}

/**
 * Name part of package change so update the name part of the main windows title
 */
void MainTab::NameChanged::text_changed(tbx::TextChangedEvent &event)
{
	std::string title = _main->title();
	std::string::size_type leaf_pos = title.rfind('.');
	std::string::size_type ver_pos = title.rfind('_');

	std::string name(event.text());

	// Make sure file name is valid
	std::string::iterator i;
	for (i = name.begin(); i != name.end(); ++i)
	   if ((*i) == '.') (*i) = '/';

	if (ver_pos == std::string::npos) return; // If not in our format don't do anything
	if (leaf_pos == std::string::npos)
	{
		title = name + title.substr(ver_pos);
	} else
	{
		title = title.substr(0, leaf_pos + 1) + name + title.substr(ver_pos);
	}
	_main->title(title);
}

/**
 * Name part of package change so update the upstream version part
 * of the main windows title
 */
void MainTab::VersionChanged::text_changed(tbx::TextChangedEvent &event)
{
	std::string title = _main->title();
	std::string::size_type ver_pos = title.rfind('_');
	std::string::size_type rp_pos = title.rfind('-');

	if (ver_pos == std::string::npos
		|| rp_pos == std::string::npos
		|| rp_pos < ver_pos) return; // If not in our format don't do anything

	std::string version(event.text());

	// Make sure file name is valid
	std::string::iterator i;
	for (i = version.begin(); i != version.end(); ++i)
	   if ((*i) == '.') (*i) = '/';

	title = title.substr(0, ver_pos+1)
		+ version + title.substr(rp_pos);

	_main->title(title);
}

/**
 * Package version has changed so update the package part of the
 * main windows title.
 */
void MainTab::RPVersionChanged::value_changed(tbx::ValueChangedEvent &event)
{
	std::string title = _main->title();
	std::string::size_type rp_pos = title.rfind('-');

	if (rp_pos == std::string::npos) return; // If not in our format don't do anything

	title = title.substr(0, rp_pos+1) + tbx::to_string(event.value());

	_main->title(title);
}
