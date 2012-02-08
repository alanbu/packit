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

#ifndef ITAB
#define ITAB

/**
 * Simple interface to all the tabs
 */
class ITab
{
   public:
     virtual ~ITab() {};

     /**
      * Called after a package is loaded, in case the tab
      * needs to take some other action to display its data
      */
     virtual void package_loaded()
     {
     }

     /**
      * Called just before saving the package, so parts of the
      * Packager class that are not updated in real time, can
      * be update before the save
      */
     virtual void pre_save()
     {

     }

     /**
      * Called when this tab has been switched to.
      *
      * Normally just used to put the focus in a field
      */
     virtual void switched_to()
     {

     }

     /**
      * Return leaf name of file with help for this tab
      */
     virtual std::string help_name() const = 0;
};

#endif
