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

/**
 * Class to handle tab to specify
 */

#ifndef DESCTAB_H
#define DESCTAB_H

#include "tbx/Window.h"
#include "tbx/textarea.h"
#include "tbx/writablefield.h"
#include "tbx/caretlistener.h"
#include "tbx/command.h"

#include "ITab.h"
#include "Packager.h"

class MainWindow;

/**
 * Tab to show description
 */
class DescTab : public ITab,
	public tbx::GainCaretListener,
	public tbx::LoseCaretListener,
	public tbx::Command
{
    Packager &_packager;
    tbx::WritableField _summary;
    tbx::TextArea _description;
    bool _has_caret;

    public:
       DescTab(MainWindow *main, tbx::Window window, Packager &packager);
       virtual ~DescTab();

       virtual void package_loaded();
       virtual void pre_save();

       virtual void switched_to();

       virtual void gain_caret(tbx::CaretEvent &event);
       virtual void lose_caret(tbx::CaretEvent &event);

       virtual void execute();

       /**
        * Return leaf name of file with help for this tab
        */
       virtual std::string help_name() const {return "description";}

};


#endif

