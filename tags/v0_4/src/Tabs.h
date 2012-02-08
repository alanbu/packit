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


#ifndef TABS_H
#define TABS_H


#include "tbx/gadget.h"
#include "tbx/listener.h"

/**
 * Listener for change of tab
 */
class TabSelectionListener : public tbx::Listener
{
public:
	TabSelectionListener() {};
	virtual ~TabSelectionListener() {};

	virtual void tab_selected(int old_index, int new_index) = 0;
};

/**
 * Class for Rick Griffins Tabs Gadget
 */
class Tabs : public tbx::Gadget
{
  public:
     enum {TOOLBOX_CLASS = 0x402c };

	Tabs() {} //!< Construct an uninitialised Tabs gadget.
	/**
	 * Destroy a Tabs gadget.
	 *
	 * This does not delete the underlying toolbox gadget.
	 */
	~Tabs() {}

	/**
	 * Construct a Tabs gadget from another Tabs gadget.
	 *
	 * Both Tabs will refer to the same underlying toolbox gadget.
	 */
	Tabs(const Tabs &other) : Gadget(other) {}

	/**
	 * Construct a Tabs gadget from another gadget.
	 *
	 * The Tabs and the Gadget will refer to the same
	 * underlying toolbox gadget.
	 *
	 * @throws GadgetClassError if the underlying toolbox gadget
	 *         isn't a Tabs gadget
	 */
	Tabs(const Gadget &other) : Gadget(other)	{check_toolbox_class(Tabs::TOOLBOX_CLASS);}

	/**
	 * Construct a Tabs gadget from a gadget.
	 *
	 * The Tabs and the Component will refer to the same
	 * underlying toolbox component.
	 */
	Tabs(const tbx::Component &other) : tbx::Gadget(other) {tbx::Window check(other.handle()); check_toolbox_class(Tabs::TOOLBOX_CLASS);}

	/**
	 * Assign a Tabs gadget to refer to the same underlying toolbox gadget
	 * as another.
	 */
	Tabs &operator=(const Tabs &other) {_handle = other._handle; _id = other._id; return *this;}

	/**
	 * Assign a Tabs gadget to refer to the same underlying toolbox gadget
	 * as an existing Gadget.
	 * @throws GadgetClassError if the underlying toolbox gadget
	 *         isn't a Tabs gadget
	 */
	Tabs &operator=(const tbx::Gadget &other) {_handle = other.handle(); _id = other.id(); check_toolbox_class(TOOLBOX_CLASS); return *this;}

	/**
	 * Assign a Tabs gadget to refer to the same underlying toolbox component
	 * as an existing Gadget.
	 * @throws ObjectClassError if the component is not in a window.
	 * @throws GadgetClassError if the underlying toolbox gadget
	 *         isn't a Tabs gadget
	 */
	Tabs &operator=(const tbx::Component &other) {tbx::Window check(other.handle()); _handle = other.handle(); _id = other.id(); check_toolbox_class(TOOLBOX_CLASS); return *this;}

	/**
	 * Check if this Tabs gadget refers to the same underlying toolbox gadget
	 * as another gadget.
	 * @returns true if they refer to the same underlying toolbox gadget.
	 */
	bool operator==(const tbx::Gadget &other) const {return (_handle == other.handle() && _id == other.id());}

	/**
	 * Check if this Tabs gadget refers to the same underlying toolbox gadget
	 * as another gadget.
	 * @returns true if they do not refer to the same underlying toolbox gadget.
	 */
	bool operator!=(const tbx::Gadget &other) const {return (_handle != other.handle() || _id != other.id());}

	/**
	 * Set the state
	 */
	void state(int value) {int_property(16428, value);}

	/**
	 * Get the state of the Tabs gadget.
	 *
	 */
	int state() const {return int_property(16429);}

	/**
	 * Set selected tab
	 *
	 * @param index 0 based index to tab to select
	 */
	 void selected(int index) {int_property(16430);}

	/**
	 * Get selected tab
	 *
	 * @return 0 based index of selected tab
	 */
	 int selected() const {return int_property(16431);}

	/**
	 * Get tab index from Window
	 *
	 * @returns index of supplied window or -1 if window not on tabs
	 */
	 int tab_from_window(const tbx::Window &window);

	/**
	 * Get Window from tab index
	 *
	 * @throws OsException if index invalid
	 */
	tbx::Window window_from_tab(int index);

	/**
	 * Set the label for the tab.
	 * By default the tab has the label of the window title.
	 */
	 void label(int index, const std::string &text);

	/**
	 * Returns the label for a specified index
	 */
	std::string label(int index) const;

	void add_selection_listener(TabSelectionListener *listener);
	void remove_selection_listener(TabSelectionListener *listener);
};

#endif
