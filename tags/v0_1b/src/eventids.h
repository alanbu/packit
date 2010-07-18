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

/*
 * eventids.h
 *
 *  Created on: 22-May-2009
 *      Author: alanb
 *
 *  User defined events from toolbox
 */

#ifndef EVENTIDS_H_
#define EVENTIDS_H_

/* Main window */
#define EID_NEXT_ERROR 1
#define EID_SAVE 2
#define EID_TAB_HELP 3

/* Whole application */
#define EID_HELP 98
#define EID_QUIT 99

/* Sys vars window */
#define EID_SYSVARS_ADD  101
#define EID_SYSVARS_EDIT 102
#define EID_SYSVARS_REMOVE 103
#define EID_SYSVARS_CLEAR 104
#define EID_SYSVARS_SCAN_BOOT 105

/* Sprites window  */
#define EID_SPRITES_ADD  101
#define EID_SPRITES_EDIT 102
#define EID_SPRITES_REMOVE 103
#define EID_SPRITES_CLEAR 104
#define EID_SPRITES_SCAN_SPRITES 105

/* Depends Window */
#define EID_ADD_DEPENDENCY 201
#endif /* EVENTIDS_H_ */
