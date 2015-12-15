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


#include "SDL_scancode.h"

#include <lair/sys_sdl2/sys_module.h>

#include "lair/ec/entity_manager.h"
#include "lair/ec/sprite_component.h"

#include "game.h"

#include "screen_state.h"


ScreenState::ScreenState(Game* game)
	: _game(game),
      _entities(_game->log()),
      _sprites(_game->renderer()),
      _running(false) {
}


void ScreenState::initialize() {
	int w = _game->window()->width();
	int h = _game->window()->height();
	glViewport(0, 0, w, h);
	_camera.setViewBox(Box3(
		Vector3(0, 0, -1),
		Vector3(w, h,  1)
	));

	_bg = _entities.createEntity(_entities.root());
	_sprites.addComponent(_bg);

	setBg("title.png");
}


void ScreenState::shutdown() {

}


void ScreenState::run() {
	_running = true;

	while(_running) {
		_game->sys()->waitAndDispatchSystemEvents();
		if(_game->sys()->getKeyState(SDL_SCANCODE_ESCAPE)) {
			_running = false;
		}
		if(_game->sys()->getKeyState(SDL_SCANCODE_LEFT)
		|| _game->sys()->getKeyState(SDL_SCANCODE_RIGHT)) {
			_game->setNextState(_game->mainState());
			_running = false;
		}

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		_game->renderer()->mainBatch().clearBuffers();

		_entities.updateWorldTransform();
		_sprites.render(0, _camera);

		_game->renderer()->spriteShader()->use();
		_game->renderer()->spriteShader()->setTextureUnit(0);
		_game->renderer()->spriteShader()->setViewMatrix(_camera.transform());
		_game->renderer()->mainBatch().render();
		_game->window()->swapBuffers();
	}
}


void ScreenState::quit() {
	_running = false;
}

void ScreenState::setBg(const std::string& bg) {
	Texture* tex = _game->renderer()->getTexture(
	            bg, Texture::BILINEAR | Texture::CLAMP);
	_sprite.reset(new Sprite(tex));

	_bg.sprite()->setSprite(_sprite.get());
	float s = float(_game->window()->height()) / tex->height();
	_bg.place(Transform(Eigen::Scaling(s, s, 1.f)));
}
