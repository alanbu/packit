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

#include "SaveCopyright.h"
#include "MainWindow.h"
#include "tbx/objectdelete.h"

tbx::SaveAs SaveCopyright::_saveas;

SaveCopyright::SaveCopyright(tbx::Object object)
{
	// Store object handle so it can be shown from code
	// As the object will be autocreate as the first main
	// window is shown it will be available when needed.
	_saveas = object;

	// Add listeners we are interested in
	_saveas.add_about_to_be_shown_listener(this);
//	_saveas.add_save_completed_listener(this);
//	_saveas.set_save_to_file_handler(this);

	_saveas.add_object_deleted_listener(new tbx::ObjectDeleteClass<SaveCopyright>(this));
}

SaveCopyright::~SaveCopyright()
{
}

/**
 * Show Save control object
 *
 * @param parent parent for save as box, ancestor of this must be the main window.
 */
void SaveCopyright::show(tbx::Window parent)
{
	_saveas.show_as_menu(parent);
}

/**
 * Get Window we want to save from
 */
void SaveCopyright::about_to_be_shown(tbx::AboutToBeShownEvent &event)
{
	tbx::SaveAs saveas(event.id_block().self_object());
	_main = MainWindow::from_window(event.id_block().ancestor_object());

    std::string copyright(_main->packager().copyright());
    saveas.set_data_address((void *)copyright.c_str(), copyright.size());
	saveas.file_name("Copyright");
}
