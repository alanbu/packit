/*********************************************************************
* Copyright 2010 Alan Buckley
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
 * CopyrightTab.cc
 *
 *  Created on: 06-Apr-2009
 *      Author: alanb
 */

#include "CopyrightTab.h"
#include "MainWindow.h"
#include "tbx/stringset.h"
#include "tbx/application.h"

CopyrightTab::CopyrightTab(MainWindow *main, tbx::Window window, Packager &packager) :
	_packager(packager)
{
	tbx::StringSet licence = window.gadget(1);
	main->set_binding(LICENCE, licence);

	// Copyright is just update on load and prior to the save
	_copyright = window.gadget(2);
	// Allow selection etc on text area
	_copyright.allow_selection(true);
	_copyright.allow_save(true);
	_copyright.allow_load(true);
	_copyright.clipboard(true);

	_copyright_empty = true;

	window.add_gain_caret_listener(this);
	window.add_lose_caret_listener(this);
}

CopyrightTab::~CopyrightTab()
{
	// Ensure idle command is always removed
	tbx::app()->remove_idle_command(this);
}

void CopyrightTab::package_loaded()
{
	std::string copytext =_packager.copyright();
	_copyright.text(copytext);
	_copyright_empty = copytext.empty();
}

void CopyrightTab::pre_save()
{
	_packager.copyright(_copyright.text());
}

/**
 * Need to add an idle handler to check copyright text
 * when this window has the caret
 */
void CopyrightTab::gain_caret(tbx::CaretEvent &event)
{
	tbx::app()->add_idle_command(this);
}

/**
 * Remove idle handler as this form no longer has the focus
 */
void CopyrightTab::lose_caret(tbx::CaretEvent &event)
{
	tbx::app()->remove_idle_command(this);
}

/**
 * Idle command processing.
 *
 * As there are no events from the text area we use an idle
 * command to check the current state of the text area.
 */
void CopyrightTab::execute()
{
	if (_packager.modified())
	{
		// Package has been modified so just check if it is empty
		if (_copyright.text_length() == 0)
		{
			if (!_copyright_empty)
			{
				_packager.copyright("");
				_copyright_empty = true;
			}
		} else if (_copyright_empty)
		{
			_packager.copyright(_copyright.text());
			_copyright_empty = false;
		}
	} else
	{
		// Package has not been modified yet so need to check
		// if copyright has been modified
		std::string copytext = _copyright.text();
		if (copytext != _packager.copyright())
		{
			_packager.copyright(copytext);
			_copyright_empty = copytext.empty();
		}
	}
}
