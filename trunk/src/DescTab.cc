/*********************************************************************
* Copyright 2009-2014 Alan Buckley
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
#include "tbx/messagewindow.h"

#include <iostream>

DescTab::DescTab(MainWindow *main, tbx::Window window, Packager &packager) :
	_packager(packager), _has_caret(false),
	_ole_edit_command(this, &DescTab::ole_edit),
	_ole_client(0)
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

	_ole_edit_button = window.gadget(3);
	_ole_edit_button.add_select_command(&_ole_edit_command);

	// Need to use null events when tab has caret so
	// modified flag can be updated
	window.add_gain_caret_listener(this);
	window.add_lose_caret_listener(this);
}

DescTab::~DescTab()
{
	// Ensure idle command is always removed
	tbx::app()->remove_idle_command(this);
	if (_ole_client) _ole_client->closed_in_client();
	delete _ole_client;
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

	if (_has_caret && _packager.modified())
	{
		// Need to restart idle command to see if a modification
		// comes in after a save
		tbx::app()->add_idle_command(this);
	}
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
	_has_caret = true;
	// Only care about updating the modified flag
	// so nothing to do it it's already set
	if (!_packager.modified())
	{
		tbx::app()->add_idle_command(this);
	}
}

void DescTab::lose_caret(tbx::CaretEvent &event)
{
	tbx::app()->remove_idle_command(this);
	_has_caret = false;
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

/**
 * Attempt to use the OLE edit protocol to edit the descripton
 */
void DescTab::ole_edit()
{
	if (_ole_client != 0)
	{
		tbx::show_message("Already editing this document");
		return;
	}
	_ole_client = new tbx::ext::OleClient(0xFFF); // Text OLE client
	if (_ole_client->find_server())
	{
		_ole_edit_button.text("OLE Edit in progress");
		_ole_edit_button.fade(true);
		_ole_client->edit_text(_description.text(), _description.window(),0,0,this);
	} else
	{
		tbx::show_message("Unable to find a Text OLE editor");
	}
}

/**
 * Ole client failed to start
 */
void DescTab::failed_to_start_server(tbx::ext::OleClient &client)
{
	tbx::show_message("Unable to start OLE edit session");
	reset_ole_edit_button();
	delete _ole_client;
	_ole_client = 0;
}

/**
 * OLE editor has been closed
 */
void DescTab::edit_closed(tbx::ext::OleClient &client)
{
	reset_ole_edit_button();
	delete _ole_client;
	_ole_client = 0;
}

/**
 * Text has been changed by the OLE editor
 */
void DescTab::edit_text_changed(tbx::ext::OleClient &client, const std::string &text)
{
	try
	{
		// Needed to ensure re-display works correctly on RISC OS 4.02
		_description.set_selection(0,0);
	} catch(...) {} // Ignore error if not implemented in used TextArea
	_description.text(text);
}

/**
 * Reset the OLE Edit button after an OLE session
 * has been closed
 */
void DescTab::reset_ole_edit_button()
{
	_ole_edit_button.text("OLE Edit...");
	_ole_edit_button.fade(false);
}
