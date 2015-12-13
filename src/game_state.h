//
//  Copyright (C) 2015 the authors (see AUTHORS)
//
//  This file is part of alice_hie.
//
//  lair is free software: you can redistribute it and/or modify it
//  under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  lair is distributed in the hope that it will be useful, but
//  WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with lair.  If not, see <http://www.gnu.org/licenses/>.
//


#ifndef _AHIE_GAME_STATE_H
#define _AHIE_GAME_STATE_H


#include <lair/core/lair.h>
#include <lair/core/log.h>


using namespace lair;


class GameState {
public:
	virtual void initialize() = 0;
	virtual void shutdown() = 0;

	virtual void run() = 0;
	virtual void quit() = 0;

protected:
};


#endif
