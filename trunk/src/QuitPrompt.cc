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
 * QuitPrompt.cc
 *
 *  Created on: 10-Jun-2009
 *      Author: alanb
 */

#include "QuitPrompt.h"
#include "Packager.h"
#include "tbx/stringutils.h"

QuitPrompt::QuitPrompt()
{
}

QuitPrompt::~QuitPrompt()
{
}

/**
 * Called when the prequit message is received by the application.
 *
 * Check if we have any modified packages and if so cancel the
 * quit and show the quit dialog box.
 */
void QuitPrompt::pre_quit(tbx::PreQuitEvent &event)
{
	unsigned int modified = modified_count();

	if (modified)
	{
		event.cancel_quit();
		_restart = event.quit_restarter();
		show_quit(modified);
	}
}

/**
 * Called when the Quit option is selected on the iconbar.
 *
 * Check if we have any modified packages and if so cancel the
 * quit and show the quit dialog box.
 */
void QuitPrompt::execute()
{
	unsigned int modified = modified_count();

	_restart.close_application_only(true);

	if (modified)
	{
		show_quit(modified);
	} else
	{
		// In this case we need to do the quit ourselves
		_restart.restart_quit();
	}
}

/**
 * Called if quit is selected on the Quit dialog box.
 *
 * Restart the cancelled quit if this happens
 */
void QuitPrompt::quit_quit()
{
	_restart.restart_quit();
}

/**
 * Show the quit dialog box
 */
void QuitPrompt::show_quit(unsigned int modified)
{
	if (_quit.null())
	{
		_quit = tbx::Quit("Quit");
		_quit.add_quit_listener(this);
	}
	std::string msg(tbx::to_string(modified) + " packages modified.");
	_quit.message(msg);
	_quit.show_as_menu();
}

/**
 * Count the number of modified packages
 */
unsigned int QuitPrompt::modified_count() const
{
	unsigned int count = 0;
	Packager *packager = Packager::first_packager();
	while (packager)
	{
		if (packager->modified()) count++;
		packager = packager->next_packager();
	}

	return count;
}
