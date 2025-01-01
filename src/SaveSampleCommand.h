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

#pragma once

#include "tbx/saveas.h"
#include "tbx/abouttobeshownlistener.h"

class MainWindow;

/**
 * Class to handle saving of the whole package
 */
class SaveSampleCommand :
	public tbx::AboutToBeShownListener
{
private:
	MainWindow *_main;
	static tbx::SaveAs _saveas;

public:
	SaveSampleCommand(tbx::Object object);
	virtual ~SaveSampleCommand();

	// Saveas dialog handling
	virtual void about_to_be_shown(tbx::AboutToBeShownEvent &event);
	static void show(tbx::Window parent);

};
