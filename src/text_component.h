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


#ifndef _LOF3_TEXT_COMPONENT_H
#define _LOF3_TEXT_COMPONENT_H


#include <lair/core/lair.h>
#include <lair/core/log.h>

#include <lair/ec/component.h>
#include <lair/ec/component_manager.h>


using namespace lair;

namespace lair {
class Renderer;
}

class Font;

class TextComponentManager;


class TextComponent : public Component {
public:
	TextComponent(_Entity* entity, SparseComponentManager<TextComponent>* manager);

	virtual void destroy();
	virtual void clone(EntityRef& target);

	Font* font;
	std::string text;
	Vector4 color;

public:
	TextComponentManager* _manager;
};


class TextComponentManager : public SparseComponentManager<TextComponent> {
public:
	void cloneComponent(EntityRef base, EntityRef entity);

	void render(float interp, Renderer* renderer);
};


#endif
