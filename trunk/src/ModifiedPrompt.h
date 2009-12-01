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
 * ModifiedPrompt.h
 *
 *  Created on: 05-Jun-2009
 *      Author: alanb
 */

#ifndef MODIFIEDPROMPT_H_
#define MODIFIEDPROMPT_H_

#include "tbx/DCS.h"

class MainWindow;

/**
 * Class to show and process the modified prompt
 */
class ModifiedPrompt : public tbx::DCSDiscardListener, public tbx::DCSSaveListener
{
private:
	tbx::DCS _dcs;
	MainWindow *_main;
	static ModifiedPrompt *_instance;

private:
	ModifiedPrompt();
	virtual ~ModifiedPrompt();

	virtual void dcs_discard();
	virtual void dcs_save();

public:
	static void show(MainWindow *main);
};

#endif /* MODIFIEDPROMPT_H_ */
