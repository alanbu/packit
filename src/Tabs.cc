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


#include "Tabs.h"

#include "tbx/swixcheck.h"

#include <memory>
#include "kernel.h"

using namespace tbx;

/**
 * Get tab index from Window
 *
 * @returns index of supplied window or -1 if window not on tabs
 */
int Tabs::tab_from_window(const tbx::Window &window)
{
   _kernel_swi_regs regs;
   regs.r[0] = 0;
   regs.r[1] = _handle;
   regs.r[2] = 16432;
   regs.r[3] = _id;
   regs.r[4] = window.handle();
   swix_check(_kernel_swi(0x44ec6, &regs, &regs));

   return regs.r[0];
}


/**
 * Get Window from tab index
 *
 * @throws OsException if index invalid
 */
Window Tabs::window_from_tab(int index)
{
   _kernel_swi_regs regs;
   regs.r[0] = 0;
   regs.r[1] = _handle;
   regs.r[2] = 16433;
   regs.r[3] = _id;
   regs.r[4] = index;
   swix_check(_kernel_swi(0x44ec6, &regs, &regs));

   return Window((ObjectId)regs.r[0]);
}


  /**
   * Set the label for the tab.
   * By default the tab has the label of the window title.
   */
void Tabs::label(int index, const std::string &text)
{
   _kernel_swi_regs regs;
   regs.r[0] = 0;
   regs.r[1] = _handle;
   regs.r[2] = 16434;
   regs.r[3] = _id;
   regs.r[4] = index;
   regs.r[5] = reinterpret_cast<int>(text.c_str());
   swix_check(_kernel_swi(0x44ec6, &regs, &regs));
}


  /**
   * Returns the label for a specified index
   */
std::string Tabs::label(int index) const
{
    _kernel_swi_regs regs;
    regs.r[0] = 0; // Flags are zero
    regs.r[1] = _handle;
    regs.r[2] = 16435;
    regs.r[3] = _id;
    regs.r[4] = index;
    regs.r[5] = 0;

    // Run Toolbox_ObjectMiscOp to get the size of the buffer
    swix_check(_kernel_swi(0x44ec6, &regs, &regs));

    std::string value;
    int len = regs.r[6];
    if (len)
    {
       std::auto_ptr<char> m_buffer(new char[len]);
       regs.r[5] = reinterpret_cast<int>(m_buffer.get());
       regs.r[6] = len;

       // Run Toolbox_ObjectMiscOp to get the buffer
       swix_check(_kernel_swi(0x44ec6, &regs, &regs));

       value = m_buffer.get();
    }

    return value;
}


/*
 * handle tab selected event
 */
static void tab_selection_handler(IdBlock &id_block, PollBlock &data, Listener *listener)
{
	int new_index = data.word[4];
	int old_index = data.word[5];
	static_cast<TabSelectionListener *>(listener)->tab_selected(old_index, new_index);
}


void Tabs::add_selection_listener(TabSelectionListener *listener)
{
	add_listener(0x1402c0, listener, tab_selection_handler);
}

void Tabs::remove_selection_listener(TabSelectionListener *listener)
{
	remove_listener(0x1402c0, listener);
}
