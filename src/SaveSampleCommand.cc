/*********************************************************************
* Copyright 2024 Alan Buckley
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

#include "SaveSampleCommand.h"
#include "MainWindow.h"
#include "tbx/objectdelete.h"

tbx::SaveAs SaveSampleCommand::_saveas;

SaveSampleCommand::SaveSampleCommand(tbx::Object object)
{
	// Store object handle so it can be shown from code
	// As the object will be autocreate as the first main
	// window is shown it will be available when needed.
	_saveas = object;

	// Add listeners we are interested in
	_saveas.add_about_to_be_shown_listener(this);

	_saveas.add_object_deleted_listener(new tbx::ObjectDeleteClass<SaveSampleCommand>(this));
}

SaveSampleCommand::~SaveSampleCommand()
{
}

/**
 * Show Save control object
 *
 * @param parent parent for save as box, ancestor of this must be the main window.
 */
void SaveSampleCommand::show(tbx::Window parent)
{
	_saveas.show_as_menu(parent);
}

/**
 * Get Window we want to save from
 */
void SaveSampleCommand::about_to_be_shown(tbx::AboutToBeShownEvent &event)
{
	tbx::SaveAs saveas(event.id_block().self_object());
	_main = MainWindow::from_window(event.id_block().ancestor_object());

	std::string item_to_package(_main->packager().item_to_package());
	if (item_to_package.find(' ') != std::string::npos)
	{
		item_to_package = "\"" + item_to_package + "\"";
	}

	std::ostringstream os;
	os << "| Sample Obey file to create a package" << std::endl;
	os << "| This assumes the Control and Copyright files are in this directory" << std::endl;
	os << "| File to pack location may need to be adjusted and should probably" << std::endl;
	os << "| be made relative to the location of this Obey file" << std::endl;
	os << std::endl;
	os << "Dir <Obey$Dir>"  << std::endl;
	os << "<PackIt$Dir>.packitcmd create Control Copyright";
	os << " pack " << item_to_package << " to " << _main->packager().install_to();
	os << std::endl;
	std::string sample(os.str());
    saveas.set_data_address((void *)sample.c_str(), sample.size());

	// Set save as properties
	saveas.file_name("CreatePkg");
}
