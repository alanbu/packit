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
 * ModifiedPrompt.cc
 *
 *  Created on: 05-Jun-2009
 *      Author: alanb
 */

#include "ModifiedPrompt.h"
#include "MainWindow.h"

ModifiedPrompt *ModifiedPrompt::_instance = 0;

ModifiedPrompt::ModifiedPrompt() : _dcs("DCS")
{
	_instance = this;

	_dcs.add_discard_listener(this);
	_dcs.add_save_listener(this);
}

ModifiedPrompt::~ModifiedPrompt()
{
}

/**
 * Static method to show DCS prompt
 */
void ModifiedPrompt::show(MainWindow *main)
{
	if (_instance == 0) new ModifiedPrompt();
	_instance->_main = main;
	_instance->_dcs.show_as_menu();
}

/**
 * Discard document
 */
void ModifiedPrompt::dcs_discard()
{
	delete _main;
}

/**
 * Save document
 */
void ModifiedPrompt::dcs_save()
{
	_main->show_save();
}
