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
#include "tbx/messagewindow.h"

CopyrightTab::CopyrightTab(MainWindow *main, tbx::Window window, Packager &packager) :
	_packager(packager),
	_ole_client(0),
	_ole_edit_command(this, &CopyrightTab::ole_edit)
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

	_ole_edit_button = window.gadget(3);
	_ole_edit_button.add_select_command(&_ole_edit_command);

	window.add_gain_caret_listener(this);
	window.add_lose_caret_listener(this);
}

CopyrightTab::~CopyrightTab()
{
	// Ensure idle command is always removed
	tbx::app()->remove_idle_command(this);
	if (_ole_client) _ole_client->closed_in_client();
	delete _ole_client;
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

/**
 * Attempt to use the OLE edit protocol to edit the descripton
 */
void CopyrightTab::ole_edit()
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
		_ole_client->edit_text(_copyright.text(), _copyright.window(),0,0,this);
	} else
	{
		tbx::show_message("Unable to find a Text OLE editor");
	}
}

/**
 * Ole client failed to start
 */
void CopyrightTab::failed_to_start_server(tbx::ext::OleClient &client)
{
	tbx::show_message("Unable to start OLE edit session");
	reset_ole_edit_button();
	delete _ole_client;
	_ole_client = 0;
}

/**
 * OLE editor has been closed
 */
void CopyrightTab::edit_closed(tbx::ext::OleClient &client)
{
	reset_ole_edit_button();
	delete _ole_client;
	_ole_client = 0;
}

/**
 * Text has been changed by the OLE editor
 */
void CopyrightTab::edit_text_changed(tbx::ext::OleClient &client, const std::string &text)
{
	try
	{
		// Needed to ensure re-display works correctly on RISC OS 4.02
		_copyright.set_selection(0,0);
	} catch(...) {} // Ignore error if not implemented in used TextArea

	_copyright.text(text);
	if (text != _packager.copyright())
	{
		_packager.copyright(text);
		_copyright_empty = text.empty();
	}
}

/**
 * Reset the OLE Edit button after an OLE session
 * has been closed
 */
void CopyrightTab::reset_ole_edit_button()
{
	_ole_edit_button.text("OLE Edit...");
	_ole_edit_button.fade(false);
}

