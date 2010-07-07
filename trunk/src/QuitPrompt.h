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
 * QuitPrompt.h
 *
 *  Created on: 10-Jun-2009
 *      Author: alanb
 */

#ifndef QUITPROMPT_H_
#define QUITPROMPT_H_

#include "tbx/prequitlistener.h"
#include "tbx/command.h"
#include "tbx/quit.h"

/**
 * Class to handle prequit message and attempt to quit from
 * the iconbar menu.
 *
 * Add it to the application as a prequit listener and as
 * a command to the menu item that does the quit;
 */
class QuitPrompt:
	public tbx::PreQuitListener,
	public tbx::Command,
	public tbx::QuitQuitListener
{
	tbx::Quit _quit;
	tbx::QuitRestarter _restart;
public:
	QuitPrompt();
	virtual ~QuitPrompt();

	virtual void pre_quit(tbx::PreQuitEvent &event);
	virtual void execute();
	virtual void quit_quit(const tbx::EventInfo &info);

	unsigned int modified_count() const;

private:
	void show_quit(unsigned int modified);
};

#endif /* QUITPROMPT_H_ */
