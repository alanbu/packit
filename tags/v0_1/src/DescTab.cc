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
 * DescTab.cc
 *
 *  Created on: 06-Apr-2009
 *      Author: alanb
 */

#include "DescTab.h"
#include "MainWindow.h"
#include "tbx/application.h"

DescTab::DescTab(MainWindow *main, tbx::Window window, Packager &packager) :
	_packager(packager)
{
	_summary = window.gadget(1);
	main->set_binding(SUMMARY, _summary);

	// Description is not updated in real time, just
	// when package is loaded or about to be saved
	_description = window.gadget(2);

	// Allow selection etc on text area
	_description.allow_selection(true);
	_description.allow_save(true);
	_description.allow_load(true);
	_description.clipboard(true);

	// Need to use null events when tab has caret so
	// modified flag can be updated
	window.add_gain_caret_listener(this);
	window.add_lose_caret_listener(this);
}

/**
 * Load description
 */
void DescTab::package_loaded()
{
	_description.text(_packager.description());
}

/**
 * Ensure description is update prior to a save
 */
void DescTab::pre_save()
{
	_packager.description(_description.text());
}

/**
 * Ensure focus is with summary tab is switched to
 */
void DescTab::switched_to()
{
	_summary.focus();
}

void DescTab::gain_caret(tbx::CaretEvent &event)
{
	// Only care about updating the modified flag
	// so nothing to do it it's already set
	if (_packager.modified())
	{
		tbx::app()->add_idle_command(this);
	}
}

void DescTab::lose_caret(tbx::CaretEvent &event)
{
	tbx::app()->remove_idle_command(this);
}

/**
 * Check if description has been modified
 */
void DescTab::execute()
{
	std::string desc = _description.text();
	if (desc != _packager.description())
	{
		// Only care about letting package know its changed
		_packager.modified(true);

		// No longer need to check
		tbx::app()->remove_idle_command(this);
	}
}
