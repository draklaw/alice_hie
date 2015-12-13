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


#ifndef _LOF3_MAIN_STATE_H
#define _LOF3_MAIN_STATE_H


#include <vector>
#include <deque>

#include <lair/core/lair.h>
#include <lair/core/log.h>
#include <lair/core/signal.h>

#include <lair/utils/interp_loop.h>
#include <lair/utils/input.h>

#include <lair/render_gl2/orthographic_camera.h>

#include <lair/ec/entity.h>
#include <lair/ec/entity_manager.h>
#include <lair/ec/sprite_component.h>

#include "menu.h"
#include "text_component.h"
#include "animation_component.h"
#include "sound_player.h"

#include "game_state.h"


using namespace lair;


class Game;

class Font;


#define MAX_FOOD 2000
#define MAX_DRINK 2000
#define START_GROWTH 1000
#define MAX_GROWTH 2000

#define DOUBLE_TAP_TIME 0.5

enum Meter { FOOD, DRINK, GROWTH };

typedef struct Effect_s Effect;
typedef struct Foodstuff_s Foodstuff;

struct Effect_s {
	Meter type;            // Which meter is affected.
	float changePerSecond; // Impact on meter (units/s).
	float effectDuration;  // Remaining duration (s).
	float totalDuration;   // Total duration (s).
	Foodstuff* source;      // Source of the effect.
};

struct Foodstuff_s {
	Meter type;                  // Should be FOOD or DRINK.
	std::vector<Effect> effects; // List of triggered effects.
};


class MainState : public GameState {
public:
	MainState(Game* game);
	~MainState();

	Sprite loadSprite(const char* file, unsigned th = 1, unsigned tv = 1);

	virtual void initialize();
	virtual void shutdown();

	virtual void run();
	virtual void quit();

	void layoutScreen();

	EntityRef createSprite(Sprite* sprite, const char* name = nullptr);
	EntityRef createSprite(Sprite* sprite, const Vector3& pos,
						   const char* name = nullptr);
	EntityRef createSprite(Sprite* sprite, const Vector3& pos,
	                       const Vector2& scale,
	                       const char* name = nullptr);
	EntityRef createText(const std::string& msg, const Vector3& pos,
	                     const Vector4& color = Vector4(1, 1, 1, 1));

	bool loadEffect(Effect* effect, const Json::Value& json);
	bool loadFood(Foodstuff* foodstuff, const Json::Value& json);
	void loadFoodSettings(const char* filename);
	void startGame();

	void updateTick();
	void updateFrame();

	Logger& log();


public:
	enum State {
		Playing,
		Dead
	};

public:
	Game* _game;

	// More or less system stuff

	EntityManager             _entities;
	SpriteComponentManager    _sprites;
	TextComponentManager      _texts;
	AnimationComponentManager _anims;
	InputManager              _inputs;

	SlotTracker _slotTracker;

	OrthographicCamera _camera;

	bool        _initialized;
	bool        _running;
	InterpLoop  _loop;
	int64       _fpsTime;
	unsigned    _fpsCount;

	Texture*    _fontTex;
	Json::Value _fontJson;
	std::unique_ptr<Font>
	            _font;

	// Game related stuff

	Input*      _drinkInput;
	Input*      _eatInput;
	Input*      _debugInput;
	float       _drinkDelay;
	float       _eatDelay;

	Sprite      _bgSprite;
	Sprite      _characterSprite;
	Sprite      _foodBarSprite;
	Sprite      _waterBarSprite;
	Sprite      _barBgSprite;

	EntityRef   _bg;
	EntityRef   _character;
	EntityRef   _foodBar;
	EntityRef   _waterBar;
	EntityRef   _foodBarBg;
	EntityRef   _waterBarBg;

	// Game states

	State       _state;

// 	std::vector<Foodstuff> _foodstuffs;
	std::vector<Foodstuff> _foodList;
	std::vector<Foodstuff> _drinkList;

	std::deque<Foodstuff> _foodQueue;
	std::deque<Foodstuff> _drinkQueue;

	float       _foodLevel;
	float       _waterLevel;
	float       _size;

	std::vector<Effect> _activeEffects;

};


#endif
