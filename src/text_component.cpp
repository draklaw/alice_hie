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


#include <lair/render_gl2/renderer.h>

#include "font.h"

#include "text_component.h"


TextComponent::TextComponent(_Entity* entity,
                             SparseComponentManager<TextComponent>* manager)
    : Component(entity),
      font(nullptr),
      text(),
      color(1, 1, 1, 1),
      _manager(static_cast<TextComponentManager*>(manager)) {
}


void TextComponent::destroy() {
	_manager->removeComponent(EntityRef(_entity()));
}


void TextComponent::clone(EntityRef& target) {
	_manager->cloneComponent(EntityRef(_entity()), target);
}


//---------------------------------------------------------------------------//


void TextComponentManager::cloneComponent(EntityRef base, EntityRef entity) {
	addComponent(entity);
	TextComponent* baseComp = get(base);
	TextComponent* comp = get(entity);
	comp->font = baseComp->font;
	comp->text = baseComp->text;
	comp->color = baseComp->color;
}


void TextComponentManager::render(float interp, Renderer* renderer) {
	_collectGarbages();
	for(auto& entityComp: *this) {
		TextComponent& comp = entityComp.second;
		if(!comp._alive) {
			continue;
		}
		if(comp.font) {
			Matrix4 wt = lerp(interp,
			                  comp._entity()->prevWorldTransform.matrix(),
			                  comp._entity()->worldTransform.matrix());
			comp.font->render(renderer, wt.block<3, 1>(0, 3), comp.color, comp.text);
		}
	}
}
