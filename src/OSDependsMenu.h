/*********************************************************************
* Copyright 2019 Alan Buckley
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
 * OSDependsMenu.h
 *
 *  Created on: 24 Mar 2019
 *      Author: alanb
 */

#ifndef OSDEPENDSMENU_H_
#define OSDEPENDSMENU_H_

#include "tbx/menu.h"
#include "tbx/autocreatelistener.h"
#include "tbx/abouttobeshownlistener.h"
#include "tbx/usereventlistener.h"

/**
 * Class to create a list of Modules in a popup menu.
 *
 * The menu must be shown from a Popup Menu gadget which has
 * and id 16 greater then the writeable field it updates.
 */
class OSDependsMenu:
	public tbx::AutoCreateListener,
	public tbx::AboutToBeShownListener,
	public tbx::UserEventListener
{
	tbx::Menu _menu;
	unsigned int _item_count;

public:
	OSDependsMenu();
	virtual ~OSDependsMenu();

	virtual void auto_created(std::string template_name, tbx::Object object);
	virtual void about_to_be_shown(tbx::AboutToBeShownEvent &event);
    virtual void user_event(tbx::UserEvent &event);

};

#endif /* OSDEPENDSMENU_H_ */
