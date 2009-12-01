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
 * Help to easily create Binary RISC OS Packaging Project packages
 *
 * Alan Buckley
 *
 * November 2008
 */

#include "tbx/application.h"
#include "tbx/iconbar.h"
#include "tbx/loader.h"
#include "tbx/autocreate.h"
#include "tbx/showhelp.h"
#include "MainWindow.h"
#include "SavePackage.h"
#include "QuitPrompt.h"
#include "DependsMenu.h"
#include "eventids.h"

/**
 * Class to load files and show main window
 */
class PackItLoader : public tbx::Loader
{
   virtual bool load_file(tbx::LoadEvent &event)
   {
       if (event.from_filer())
       {
    	   printf("from filer\n");
          MainWindow *main = new MainWindow();
          main->show();
          printf("after show\n");
          if (!main->load_file(event.file_name(), event.file_type()))
          {
        	  delete main;
          }
          printf("after load file\n");
          return true;
       }

       return false;
   }

};


int main(int argc, char *argv[])
{
	tbx::Application packit_app("<PackIt$Dir>");
	tbx::Iconbar iconbar("IconbarIcon");
	QuitPrompt quit_prompt;
	tbx::ShowHelp show_help;

	packit_app.add_prequit_listener(&quit_prompt);
	packit_app.add_command(EID_HELP, &show_help);

	iconbar.add_command(EID_QUIT, &quit_prompt);

	packit_app.set_autocreate_listener("SaveAs", new tbx::AutoCreateClass<SavePackage>());
    packit_app.set_autocreate_listener("DependsMenu", new DependsMenu());

	iconbar.add_loader(new PackItLoader());

    // Menu for list of dependencies

	iconbar.show();
	packit_app.run();

	return 0;
}
