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
 * SavePackage.h
 *
 *  Created on: 08-Apr-2009
 *      Author: alanb
 */

#ifndef SAVEPACKAGE_H_
#define SAVEPACKAGE_H_

#include "tbx/saveas.h"
#include "tbx/abouttobeshownlistener.h"

class MainWindow;

/**
 * Class to handle saving of the whole package
 */
class SavePackage :
	public tbx::AboutToBeShownListener,
	public tbx::SaveAsSaveToFileHandler,
	public tbx::SaveAsSaveCompletedListener
{
private:
	MainWindow *_main;
	static tbx::SaveAs _saveas;

public:
	SavePackage(tbx::Object object);
	virtual ~SavePackage();

	// Saveas dialog handling
	virtual void about_to_be_shown(tbx::AboutToBeShownEvent &event);
	virtual void saveas_save_to_file(tbx::SaveAs saveas, bool selection, std::string filename);
	virtual void saveas_save_completed(tbx::SaveAsSaveCompletedEvent &event);

	static void show(tbx::Window parent);

};

#endif /* SAVEPACKAGE_H_ */
