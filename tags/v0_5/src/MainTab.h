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
 * Class to handle tab to specify main details for package.
 *
 * i.e. name, version etc
 */

#ifndef MAINTAB_H
#define MAINTAB_H

#include "tbx/window.h"
#include "tbx/writablefield.h"
#include "tbx/stringset.h"
#include "tbx/numberrange.h"
#include "tbx/textchangedlistener.h"
#include "tbx/valuechangedlistener.h"
#include "ITab.h"
#include "Packager.h"

class MainWindow;

class MainTab : public ITab
{
    Packager &_packager;
    tbx::WritableField _initial_focus;

    /**
     * Class to keep main windows title name part in line
     * with package name
     */
    class NameChanged : public tbx::TextChangedListener
    {
    private:
    	MainWindow *_main;
    public:
    	NameChanged(MainWindow *main) : _main(main) {};

    	virtual void text_changed(tbx::TextChangedEvent &event);
    } _name_changed;

    /**
     * Class to keep main windows title version part in line
     * with package version
     */
    class VersionChanged : public tbx::TextChangedListener
    {
    private:
    	MainWindow *_main;
    public:
    	VersionChanged(MainWindow *main) : _main(main) {};

    	virtual void text_changed(tbx::TextChangedEvent &event);
    } _version_changed;


    /**
     * Class to keep main windows title RiscPkg version part in line
     * with RiscPkg version
     */
    class RPVersionChanged : public tbx::ValueChangedListener
    {
    private:
    	MainWindow *_main;
    public:
    	RPVersionChanged(MainWindow *main) : _main(main) {};

		virtual void value_changed(tbx::ValueChangedEvent &event);
    } _rpversion_changed;


public:
   MainTab(MainWindow *main, tbx::Window window, Packager &packager);
   virtual ~MainTab() {}

   virtual void package_loaded();

   virtual void switched_to();

   /**
    * Return leaf name of file with help for this tab
    */
   virtual std::string help_name() const {return "main";}

};


#endif
